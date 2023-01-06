#include "stdafx.h"
#include "glemu.h"

//extern int intel_mapbufferrange_bug;

namespace gle
{
	struct attribinfo
	{
		int type, size, formatsize, offset;
		GLenum format;

		attribinfo() : type(0), size(0), formatsize(0), offset(0), format(GL_FALSE) {}

		bool operator==(const attribinfo& a) const
		{
			return type == a.type && size == a.size && format == a.format && offset == a.offset;
		}
		bool operator!=(const attribinfo& a) const
		{
			return type != a.type || size != a.size || format != a.format || offset != a.offset;
		}
	};

	extern const char* const attribnames[MAXATTRIBS] = { "vvertex", "vcolor", "vtexcoord0", "vtexcoord1", "vnormal", "vtangent", "vboneweight", "vboneindex" };
	ucharbuf attribbuf;
	static uchar* attribdata;
	static attribinfo attribdefs[MAXATTRIBS], lastattribs[MAXATTRIBS];
	int enabled = 0;
	static int numattribs = 0, attribmask = 0, numlastattribs = 0, lastattribmask = 0, vertexsize = 0, lastvertexsize = 0;
	static GLenum primtype = GL_TRIANGLES;
	static uchar* lastbuf = NULL;
	static bool changedattribs = false;
	static Vector<GLint> multidrawstart;
	static Vector<GLsizei> multidrawcount;

#define MAXQUADS (0x10000/4)
	static GLuint quadindexes = 0;
	static bool quadsenabled = false;

#define MAXVBOSIZE (4*1024*1024)
	static GLuint vbo = 0;
	static int vbooffset = MAXVBOSIZE;

	static GLuint defaultvao = 0;

	void enablequads()
	{
		quadsenabled = true;

		if (quadindexes)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadindexes);
			return;
		}

		glGenBuffers(1, &quadindexes);
		ushort* data = new ushort[MAXQUADS * 6], * dst = data;
		for (int idx = 0; idx < MAXQUADS * 4; idx += 4, dst += 6)
		{
			dst[0] = idx;
			dst[1] = idx + 1;
			dst[2] = idx + 2;
			dst[3] = idx + 0;
			dst[4] = idx + 2;
			dst[5] = idx + 3;
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadindexes);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAXQUADS * 6 * sizeof(ushort), data, GL_STATIC_DRAW);
		delete[] data;
	}

	void disablequads()
	{
		quadsenabled = false;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void drawquads(int offset, int count)
	{
		if (count <= 0) return;
		if (offset + count > MAXQUADS)
		{
			if (offset >= MAXQUADS) return;
			count = MAXQUADS - offset;
		}
		glDrawRangeElements(GL_TRIANGLES, offset * 4, (offset + count) * 4 - 1, count * 6, GL_UNSIGNED_SHORT, (ushort*)0 + offset * 6);
	}

	void defattrib(int type, int size, int format)
	{
		if (type == ATTRIB_VERTEX)
		{
			numattribs = attribmask = 0;
			vertexsize = 0;
		}
		changedattribs = true;
		attribmask |= 1 << type;
		attribinfo& a = attribdefs[numattribs++];
		a.type = type;
		a.size = size;
		a.format = format;
		switch (format)
		{
		case 'B': case GL_UNSIGNED_BYTE:  a.formatsize = 1; a.format = GL_UNSIGNED_BYTE; break;
		case 'b': case GL_BYTE:           a.formatsize = 1; a.format = GL_BYTE; break;
		case 'S': case GL_UNSIGNED_SHORT: a.formatsize = 2; a.format = GL_UNSIGNED_SHORT; break;
		case 's': case GL_SHORT:          a.formatsize = 2; a.format = GL_SHORT; break;
		case 'I': case GL_UNSIGNED_INT:   a.formatsize = 4; a.format = GL_UNSIGNED_INT; break;
		case 'i': case GL_INT:            a.formatsize = 4; a.format = GL_INT; break;
		case 'f': case GL_FLOAT:          a.formatsize = 4; a.format = GL_FLOAT; break;
		case 'd': case GL_DOUBLE:         a.formatsize = 8; a.format = GL_DOUBLE; break;
		default:                          a.formatsize = 0; a.format = GL_FALSE; break;
		}
		a.formatsize *= size;
		a.offset = vertexsize;
		vertexsize += a.formatsize;
	}

	void defattribs(const char* fmt)
	{
		for (;; fmt += 3)
		{
			GLenum format;
			switch (fmt[0])
			{
			case 'v': format = ATTRIB_VERTEX; break;
			case 'c': format = ATTRIB_COLOR; break;
			case 't': format = ATTRIB_TEXCOORD0; break;
			case 'T': format = ATTRIB_TEXCOORD1; break;
			case 'n': format = ATTRIB_NORMAL; break;
			case 'x': format = ATTRIB_TANGENT; break;
			case 'w': format = ATTRIB_BONEWEIGHT; break;
			case 'i': format = ATTRIB_BONEINDEX; break;
			default: return;
			}
			defattrib(format, fmt[1] - '0', fmt[2]);
		}
	}

	static inline void setattrib(const attribinfo& a, uchar* buf)
	{
		switch (a.type)
		{
		case ATTRIB_VERTEX:
		case ATTRIB_TEXCOORD0:
		case ATTRIB_TEXCOORD1:
		case ATTRIB_BONEINDEX:
			glVertexAttribPointer(a.type, a.size, a.format, GL_FALSE, vertexsize, buf);
			break;
		case ATTRIB_COLOR:
		case ATTRIB_NORMAL:
		case ATTRIB_TANGENT:
		case ATTRIB_BONEWEIGHT:
			glVertexAttribPointer(a.type, a.size, a.format, GL_TRUE, vertexsize, buf);
			break;
		}
		if (!(enabled & (1 << a.type)))
		{
			glEnableVertexAttribArray(a.type);
			enabled |= 1 << a.type;
		}
	}

	static inline void unsetattrib(const attribinfo& a)
	{
		glDisableVertexAttribArray(a.type);
		enabled &= ~(1 << a.type);
	}

	static inline void setattribs(uchar* buf)
	{
		bool forceattribs = numattribs != numlastattribs || vertexsize != lastvertexsize || buf != lastbuf;
		if (forceattribs || changedattribs)
		{
			int diffmask = enabled & lastattribmask & ~attribmask;
			if (diffmask) loopi(numlastattribs)
			{
				const attribinfo& a = lastattribs[i];
				if (diffmask & (1 << a.type)) unsetattrib(a);
			}
			uchar* src = buf;
			loopi(numattribs)
			{
				const attribinfo& a = attribdefs[i];
				if (forceattribs || a != lastattribs[i])
				{
					setattrib(a, src);
					lastattribs[i] = a;
				}
				src += a.formatsize;
			}
			lastbuf = buf;
			numlastattribs = numattribs;
			lastattribmask = attribmask;
			lastvertexsize = vertexsize;
			changedattribs = false;
		}
	}

	void begin(GLenum mode)
	{
		primtype = mode;
	}

	void begin(GLenum mode, int numverts)
	{
		primtype = mode;
		//if (!intel_mapbufferrange_bug)
		{
			int len = numverts * vertexsize;
			if (vbooffset + len >= MAXVBOSIZE)
			{
				len = Min(len, MAXVBOSIZE);
				if (!vbo) glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, MAXVBOSIZE, NULL, GL_STREAM_DRAW);
				vbooffset = 0;
			}
			else if (!lastvertexsize) glBindBuffer(GL_ARRAY_BUFFER, vbo);
			void* buf = glMapBufferRange(GL_ARRAY_BUFFER, vbooffset, len, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
			if (buf) attribbuf.Reset((uchar*)buf, len);
		}
	}

	void multidraw()
	{
		int start = multidrawstart.Length() ? multidrawstart.Last() + multidrawcount.Last() : 0,
			count = attribbuf.Length() / vertexsize - start;
		if (count > 0)
		{
			multidrawstart.Add(start);
			multidrawcount.Add(count);
		}
	}

	int end()
	{
		uchar* buf = attribbuf.GetBuf();
		if (attribbuf.Empty())
		{
			if (buf != attribdata)
			{
				glUnmapBuffer(GL_ARRAY_BUFFER);
				attribbuf.Reset(attribdata, MAXVBOSIZE);
			}
			return 0;
		}
		int start = 0;
		{
			if (buf == attribdata)
			{
				if (vbooffset + attribbuf.Length() >= MAXVBOSIZE)
				{
					if (!vbo) glGenBuffers(1, &vbo);
					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glBufferData(GL_ARRAY_BUFFER, MAXVBOSIZE, NULL, GL_STREAM_DRAW);
					vbooffset = 0;
				}
				else if (!lastvertexsize) glBindBuffer(GL_ARRAY_BUFFER, vbo);
				void* dst = /*intel_mapbufferrange_bug ? NULL :*/
					glMapBufferRange(GL_ARRAY_BUFFER, vbooffset, attribbuf.Length(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
				if (dst)
				{
					memcpy(dst, attribbuf.GetBuf(), attribbuf.Length());
					glUnmapBuffer(GL_ARRAY_BUFFER);
				}
				else glBufferSubData(GL_ARRAY_BUFFER, vbooffset, attribbuf.Length(), attribbuf.GetBuf());
			}
			else glUnmapBuffer(GL_ARRAY_BUFFER);
			buf = (uchar*)0 + vbooffset;
			if (vertexsize == lastvertexsize && buf >= lastbuf)
			{
				start = int(buf - lastbuf) / vertexsize;
				if (primtype == GL_QUADS && (start % 4 || start + attribbuf.Length() / vertexsize >= 4 * MAXQUADS))
					start = 0;
				else buf = lastbuf;
			}
			vbooffset += attribbuf.Length();
		}
		setattribs(buf);
		int numvertexes = attribbuf.Length() / vertexsize;
		if (primtype == GL_QUADS)
		{
			if (!quadsenabled) enablequads();
			for (int quads = numvertexes / 4;;)
			{
				int count = Min(quads, MAXQUADS);
				drawquads(start / 4, count);
				quads -= count;
				if (quads <= 0) break;
				setattribs(buf + 4 * count * vertexsize);
				start = 0;
			}
		}
		else
		{
			if (multidrawstart.Length())
			{
				multidraw();
				if (start) loopv(multidrawstart) multidrawstart[i] += start;
				glMultiDrawArrays(primtype, multidrawstart.GetBuf(), multidrawcount.GetBuf(), multidrawstart.Length());
				multidrawstart.SetSize(0);
				multidrawcount.SetSize(0);
			}
			else glDrawArrays(primtype, start, numvertexes);
		}
		attribbuf.Reset(attribdata, MAXVBOSIZE);
		return numvertexes;
	}

	void forcedisable()
	{
		for (int i = 0; enabled; i++) if (enabled & (1 << i)) { glDisableVertexAttribArray(i); enabled &= ~(1 << i); }
		numlastattribs = lastattribmask = lastvertexsize = 0;
		lastbuf = NULL;
		if (quadsenabled) disablequads();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void setup()
	{
		if (!defaultvao) glGenVertexArrays(1, &defaultvao);
		glBindVertexArray(defaultvao);
		attribdata = new uchar[MAXVBOSIZE];
		attribbuf.Reset(attribdata, MAXVBOSIZE);
	}

	void cleanup()
	{
		disable();

		if (quadindexes) { glDeleteBuffers(1, &quadindexes); quadindexes = 0; }
		if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
		vbooffset = MAXVBOSIZE;
		if (defaultvao) { glDeleteVertexArrays(1, &defaultvao); defaultvao = 0; }
	}
}