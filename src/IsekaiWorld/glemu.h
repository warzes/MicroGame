#pragma once

#include "Geom.h"

namespace gle
{
	enum
	{
		ATTRIB_VERTEX = 0,
		ATTRIB_COLOR = 1,
		ATTRIB_TEXCOORD0 = 2,
		ATTRIB_TEXCOORD1 = 3,
		ATTRIB_NORMAL = 4,
		ATTRIB_TANGENT = 5,
		ATTRIB_BONEWEIGHT = 6,
		ATTRIB_BONEINDEX = 7,
		MAXATTRIBS = 8
	};

	extern const char* const attribnames[MAXATTRIBS];
	extern ucharbuf attribbuf;

	extern int enabled;
	extern void forcedisable();
	static inline void disable() { if (enabled) forcedisable(); }

	extern void begin(GLenum mode);
	extern void begin(GLenum mode, int numverts);
	extern void defattribs(const char* fmt);
	extern void defattrib(int type, int size, int format);

#define GLE_DEFATTRIB(name, type, defaultsize, defaultformat) \
	static inline void def##name(int size = defaultsize, int format = defaultformat) { defattrib(type, size, format); }

	GLE_DEFATTRIB(vertex, ATTRIB_VERTEX, 3, GL_FLOAT)
	GLE_DEFATTRIB(color, ATTRIB_COLOR, 3, GL_FLOAT)
	GLE_DEFATTRIB(texcoord0, ATTRIB_TEXCOORD0, 2, GL_FLOAT)
	GLE_DEFATTRIB(texcoord1, ATTRIB_TEXCOORD1, 2, GL_FLOAT)
	GLE_DEFATTRIB(normal, ATTRIB_NORMAL, 3, GL_FLOAT)
	GLE_DEFATTRIB(tangent, ATTRIB_TANGENT, 4, GL_FLOAT)
	GLE_DEFATTRIB(boneweight, ATTRIB_BONEWEIGHT, 4, GL_UNSIGNED_BYTE)
	GLE_DEFATTRIB(boneindex, ATTRIB_BONEINDEX, 4, GL_UNSIGNED_BYTE)

#define GLE_INITATTRIB(name, index, suffix, type) \
	static inline void name##suffix(type x) { glVertexAttrib1##suffix##(index, x); } \
	static inline void name##suffix(type x, type y) { glVertexAttrib2##suffix##(index, x, y); } \
	static inline void name##suffix(type x, type y, type z) { glVertexAttrib3##suffix##(index, x, y, z); } \
	static inline void name##suffix(type x, type y, type z, type w) { glVertexAttrib4##suffix##(index, x, y, z, w); }

#define GLE_INITATTRIBF(name, index) \
	GLE_INITATTRIB(name, index, f, float) \
	static inline void name(const vec &v) { glVertexAttrib3fv(index, v.v); } \
	static inline void name(const vec &v, float w) { glVertexAttrib4f(index, v.x, v.y, v.z, w); } \
	static inline void name(const vec2 &v) { glVertexAttrib2fv(index, v.v); } \
	static inline void name(const vec4 &v) { glVertexAttrib4fv(index, v.v); }
#define GLE_INITATTRIBN(name, index, suffix, type, defaultw) \
	static inline void name##suffix(type x, type y, type z, type w = defaultw) { glVertexAttrib4N##suffix##(index, x, y, z, w); }

	GLE_INITATTRIBF(vertex, ATTRIB_VERTEX)
	GLE_INITATTRIBF(color, ATTRIB_COLOR)
	static inline void color(const bvec4& v) { glVertexAttrib4Nubv(ATTRIB_COLOR, v.v); }
	static inline void color(const bvec& v, uchar alpha = 255) { color(bvec4(v, alpha)); }
	static inline void colorub(uchar x, uchar y, uchar z, uchar w = 255) { color(bvec4(x, y, z, w)); }
	GLE_INITATTRIBF(texcoord0, ATTRIB_TEXCOORD0)
	GLE_INITATTRIBF(texcoord1, ATTRIB_TEXCOORD1)
	static inline void normal(float x, float y, float z) { glVertexAttrib4f(ATTRIB_NORMAL, x, y, z, 0.0f); }
	static inline void normal(const vec& v) { glVertexAttrib4f(ATTRIB_NORMAL, v.x, v.y, v.z, 0.0f); }
	static inline void tangent(float x, float y, float z, float w = 1.0f) { glVertexAttrib4f(ATTRIB_TANGENT, x, y, z, w); }
	static inline void tangent(const vec& v, float w = 1.0f) { glVertexAttrib4f(ATTRIB_TANGENT, v.x, v.y, v.z, w); }
	static inline void tangent(const vec4& v) { glVertexAttrib4fv(ATTRIB_TANGENT, v.v); }

#define GLE_ATTRIBPOINTER(name, index, defaultnormalized, defaultsize, defaulttype, prepare) \
		static inline void enable##name() { prepare; glEnableVertexAttribArray(index); } \
		static inline void disable##name() { glDisableVertexAttribArray(index); } \
		static inline void name##pointer(int stride, const void *data, GLenum type = defaulttype, int size = defaultsize, GLenum normalized = defaultnormalized) { \
		prepare; \
		glVertexAttribPointer(index, size, type, normalized, stride, data); \
	}

	static inline void enableattrib(int index) { disable(); glEnableVertexAttribArray(index); }
	static inline void disableattrib(int index) { glDisableVertexAttribArray(index); }
	GLE_ATTRIBPOINTER(vertex, ATTRIB_VERTEX, GL_FALSE, 3, GL_FLOAT, disable())
	GLE_ATTRIBPOINTER(color, ATTRIB_COLOR, GL_TRUE, 4, GL_UNSIGNED_BYTE, )
	GLE_ATTRIBPOINTER(texcoord0, ATTRIB_TEXCOORD0, GL_FALSE, 2, GL_FLOAT, )
	GLE_ATTRIBPOINTER(texcoord1, ATTRIB_TEXCOORD1, GL_FALSE, 2, GL_FLOAT, )
	GLE_ATTRIBPOINTER(normal, ATTRIB_NORMAL, GL_TRUE, 3, GL_FLOAT, )
	GLE_ATTRIBPOINTER(tangent, ATTRIB_TANGENT, GL_TRUE, 4, GL_FLOAT, )
	GLE_ATTRIBPOINTER(boneweight, ATTRIB_BONEWEIGHT, GL_TRUE, 4, GL_UNSIGNED_BYTE, )
	GLE_ATTRIBPOINTER(boneindex, ATTRIB_BONEINDEX, GL_FALSE, 4, GL_UNSIGNED_BYTE, )

		static inline void bindebo(GLuint ebo) { disable(); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); }
	static inline void clearebo() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
	static inline void bindvbo(GLuint vbo) { disable(); glBindBuffer(GL_ARRAY_BUFFER, vbo); }
	static inline void clearvbo() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

	template<class T>
	static inline void attrib(T x)
	{
		if (attribbuf.Check(sizeof(T)))
		{
			T* buf = (T*)attribbuf.Pad(sizeof(T));
			buf[0] = x;
		}
	}

	template<class T>
	static inline void attrib(T x, T y)
	{
		if (attribbuf.Check(2 * sizeof(T)))
		{
			T* buf = (T*)attribbuf.Pad(2 * sizeof(T));
			buf[0] = x;
			buf[1] = y;
		}
	}

	template<class T>
	static inline void attrib(T x, T y, T z)
	{
		if (attribbuf.Check(3 * sizeof(T)))
		{
			T* buf = (T*)attribbuf.Pad(3 * sizeof(T));
			buf[0] = x;
			buf[1] = y;
			buf[2] = z;
		}
	}

	template<class T>
	static inline void attrib(T x, T y, T z, T w)
	{
		if (attribbuf.Check(4 * sizeof(T)))
		{
			T* buf = (T*)attribbuf.Pad(4 * sizeof(T));
			buf[0] = x;
			buf[1] = y;
			buf[2] = z;
			buf[3] = w;
		}
	}

	template<size_t N, class T>
	static inline void attribv(const T* v)
	{
		attribbuf.Put((const uchar*)v, N * sizeof(T));
	}

#define GLE_ATTRIB(suffix, type) \
	static inline void attrib##suffix(type x) { attrib<type>(x); } \
	static inline void attrib##suffix(type x, type y) { attrib<type>(x, y); } \
	static inline void attrib##suffix(type x, type y, type z) { attrib<type>(x, y, z); } \
	static inline void attrib##suffix(type x, type y, type z, type w) { attrib<type>(x, y, z, w); }

	GLE_ATTRIB(f, float)
	GLE_ATTRIB(d, double)
	GLE_ATTRIB(b, char)
	GLE_ATTRIB(ub, uchar)
	GLE_ATTRIB(s, short)
	GLE_ATTRIB(us, ushort)
	GLE_ATTRIB(i, int)
	GLE_ATTRIB(ui, uint)

	static inline void attrib(const vec& v) { attribf(v.x, v.y, v.z); }
	static inline void attrib(const vec& v, float w) { attribf(v.x, v.y, v.z, w); }
	static inline void attrib(const vec2& v) { attribf(v.x, v.y); }
	static inline void attrib(const vec4& v) { attribf(v.x, v.y, v.z, v.w); }
	static inline void attrib(const ivec& v) { attribi(v.x, v.y, v.z); }
	static inline void attrib(const ivec& v, int w) { attribi(v.x, v.y, v.z, w); }
	static inline void attrib(const ivec2& v) { attribi(v.x, v.y); }
	static inline void attrib(const ivec4& v) { attribi(v.x, v.y, v.z, v.w); }
	static inline void attrib(const bvec& b) { attribub(b.x, b.y, b.z); }
	static inline void attrib(const bvec& b, uchar w) { attribub(b.x, b.y, b.z, w); }
	static inline void attrib(const bvec4& b) { attribub(b.x, b.y, b.z, b.w); }

	extern void multidraw();
	extern int end();

	extern void enablequads();
	extern void disablequads();
	extern void drawquads(int offset, int count);

	extern void setup();
	extern void cleanup();
}