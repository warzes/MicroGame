#include "stdafx.h"
#include "Streams.h"
#include "FileSystem.h"
#include "utf8Conv.h"
#include "ZipFile.h"

stream::offset stream::size()
{
	offset pos = tell(), endpos;
	if (pos < 0 || !seek(0, SEEK_END)) return -1;
	endpos = tell();
	return pos == endpos || seek(pos, SEEK_SET) ? endpos : -1;
}

bool stream::getline(char* str, size_t len)
{
	loopi(len - 1)
	{
		if (read(&str[i], 1) != 1) { str[i] = '\0'; return i > 0; }
		else if (str[i] == '\n') { str[i + 1] = '\0'; return true; }
	}
	if (len > 0) str[len - 1] = '\0';
	return true;
}

size_t stream::printf(const char* fmt, ...)
{
	char buf[512];
	char* str = buf;
	va_list args;
#if defined(_WIN32) && !defined(__GNUC__)
	va_start(args, fmt);
	int len = _vscprintf(fmt, args);
	if (len <= 0) { va_end(args); return 0; }
	if (len >= (int)sizeof(buf)) str = new char[len + 1];
	_vsnprintf(str, len + 1, fmt, args);
	va_end(args);
#else
	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	if (len <= 0) return 0;
	if (len >= (int)sizeof(buf))
	{
		str = new char[len + 1];
		va_start(args, fmt);
		vsnprintf(str, len + 1, fmt, args);
		va_end(args);
	}
#endif
	size_t n = write(str, len);
	if (str != buf) delete[] str;
	return n;
}

struct filestream : stream
{
	FILE* file;

	filestream() : file(NULL) {}
	~filestream() { close(); }

	bool open(const char* name, const char* mode)
	{
		if (file) return false;
		file = fopen(name, mode);
		return file != NULL;
	}

	bool opentemp(const char* name, const char* mode)
	{
		if (file) return false;
#ifdef _WIN32
		file = fopen(name, mode);
#else
		file = tmpfile();
#endif
		return file != NULL;
	}

	void close()
	{
		if (file) { fclose(file); file = NULL; }
	}

	bool end() { return feof(file) != 0; }
	offset tell()
	{
#ifdef _WIN32
#if defined(__GNUC__) && !defined(__MINGW32__)
		offset off = ftello64(file);
#else
		offset off = _ftelli64(file);
#endif
#else
		offset off = ftello(file);
#endif
		// ftello returns LONG_MAX for directories on some platforms
		return off + 1 >= 0 ? off : -1;
	}
	bool seek(offset pos, int whence)
	{
#ifdef _WIN32
#if defined(__GNUC__) && !defined(__MINGW32__)
		return fseeko64(file, pos, whence) >= 0;
#else
		return _fseeki64(file, pos, whence) >= 0;
#endif
#else
		return fseeko(file, pos, whence) >= 0;
#endif
	}

	size_t read(void* buf, size_t len) { return fread(buf, 1, len, file); }
	size_t write(const void* buf, size_t len) { return fwrite(buf, 1, len, file); }
	bool flush() { return !fflush(file); }
	int getchar() { return fgetc(file); }
	bool putchar(int c) { return fputc(c, file) != EOF; }
	bool getline(char* str, size_t len) { return fgets(str, len, file) != NULL; }
	bool putstring(const char* str) { return fputs(str, file) != EOF; }

	size_t printf(const char* fmt, ...)
	{
		va_list v;
		va_start(v, fmt);
		int result = vfprintf(file, fmt, v);
		va_end(v);
		return Max(result, 0);
	}
};

struct gzstream : stream
{
	enum
	{
		MAGIC1 = 0x1F,
		MAGIC2 = 0x8B,
		BUFSIZE = 16384,
		OS_UNIX = 0x03
	};

	enum
	{
		F_ASCII = 0x01,
		F_CRC = 0x02,
		F_EXTRA = 0x04,
		F_NAME = 0x08,
		F_COMMENT = 0x10,
		F_RESERVED = 0xE0
	};

	stream* file;
	z_stream zfile;
	uchar* buf;
	bool reading, writing, autoclose;
	uint crc;
	size_t headersize;

	gzstream() : file(NULL), buf(NULL), reading(false), writing(false), autoclose(false), crc(0), headersize(0)
	{
		zfile.zalloc = NULL;
		zfile.zfree = NULL;
		zfile.opaque = NULL;
		zfile.next_in = zfile.next_out = NULL;
		zfile.avail_in = zfile.avail_out = 0;
	}

	~gzstream()
	{
		close();
	}

	void writeheader()
	{
		uchar header[] = { MAGIC1, MAGIC2, Z_DEFLATED, 0, 0, 0, 0, 0, 0, OS_UNIX };
		file->write(header, sizeof(header));
	}

	void readbuf(size_t size = BUFSIZE)
	{
		if (!zfile.avail_in) zfile.next_in = (Bytef*)buf;
		size = std::min(size, size_t(&buf[BUFSIZE] - &zfile.next_in[zfile.avail_in]));
		size_t n = file->read(zfile.next_in + zfile.avail_in, size);
		if (n > 0) zfile.avail_in += n;
	}

	uchar readbyte(size_t size = BUFSIZE)
	{
		if (!zfile.avail_in) readbuf(size);
		if (!zfile.avail_in) return 0;
		zfile.avail_in--;
		return *(uchar*)zfile.next_in++;
	}

	void skipbytes(size_t n)
	{
		while (n > 0 && zfile.avail_in > 0)
		{
			size_t skipped = std::min(n, size_t(zfile.avail_in));
			zfile.avail_in -= skipped;
			zfile.next_in += skipped;
			n -= skipped;
		}
		if (n <= 0) return;
		file->seek(n, SEEK_CUR);
	}

	bool checkheader()
	{
		readbuf(10);
		if (readbyte() != MAGIC1 || readbyte() != MAGIC2 || readbyte() != Z_DEFLATED) return false;
		uchar flags = readbyte();
		if (flags & F_RESERVED) return false;
		skipbytes(6);
		if (flags & F_EXTRA)
		{
			size_t len = readbyte(512);
			len |= size_t(readbyte(512)) << 8;
			skipbytes(len);
		}
		if (flags & F_NAME) while (readbyte(512));
		if (flags & F_COMMENT) while (readbyte(512));
		if (flags & F_CRC) skipbytes(2);
		headersize = size_t(file->tell() - zfile.avail_in);
		return zfile.avail_in > 0 || !file->end();
	}

	bool open(stream* f, const char* mode, bool needclose, int level)
	{
		if (file) return false;
		for (; *mode; mode++)
		{
			if (*mode == 'r') { reading = true; break; }
			else if (*mode == 'w') { writing = true; break; }
		}
		if (reading)
		{
			if (inflateInit2(&zfile, -MAX_WBITS) != Z_OK) reading = false;
		}
		else if (writing && deflateInit2(&zfile, level, Z_DEFLATED, -MAX_WBITS, Min(MAX_MEM_LEVEL, 8), Z_DEFAULT_STRATEGY) != Z_OK) writing = false;
		if (!reading && !writing) return false;

		file = f;
		crc = crc32(0, NULL, 0);
		buf = new uchar[BUFSIZE];

		if (reading)
		{
			if (!checkheader()) { stopreading(); return false; }
		}
		else if (writing) writeheader();

		autoclose = needclose;
		return true;
	}

	uint getcrc() { return crc; }

	void finishreading()
	{
		if (!reading) return;
		/*if (dbggz)
		{
			uint checkcrc = 0, checksize = 0;
			loopi(4) checkcrc |= uint(readbyte()) << (i * 8);
			loopi(4) checksize |= uint(readbyte()) << (i * 8);
			if (checkcrc != crc)
				conoutf(CON_DEBUG, "gzip crc check failed: read %X, calculated %X", checkcrc, crc);
			if (checksize != zfile.total_out)
				conoutf(CON_DEBUG, "gzip size check failed: read %u, calculated %u", checksize, uint(zfile.total_out));
		}*/
	}

	void stopreading()
	{
		if (!reading) return;
		inflateEnd(&zfile);
		reading = false;
	}

	void finishwriting()
	{
		if (!writing) return;
		for (;;)
		{
			int err = zfile.avail_out > 0 ? deflate(&zfile, Z_FINISH) : Z_OK;
			if (err != Z_OK && err != Z_STREAM_END) break;
			flushbuf();
			if (err == Z_STREAM_END) break;
		}
		uchar trailer[8] =
		{
		uchar(crc & 0xFF), uchar((crc >> 8) & 0xFF), uchar((crc >> 16) & 0xFF), uchar((crc >> 24) & 0xFF),
		uchar(zfile.total_in & 0xFF), uchar((zfile.total_in >> 8) & 0xFF), uchar((zfile.total_in >> 16) & 0xFF), uchar((zfile.total_in >> 24) & 0xFF)
		};
		file->write(trailer, sizeof(trailer));
	}

	void stopwriting()
	{
		if (!writing) return;
		deflateEnd(&zfile);
		writing = false;
	}

	void close()
	{
		if (reading) finishreading();
		stopreading();
		if (writing) finishwriting();
		stopwriting();
		DELETEA(buf);
		if (autoclose) DELETEP(file);
	}

	bool end() { return !reading && !writing; }
	offset tell() { return reading ? zfile.total_out : (writing ? zfile.total_in : offset(-1)); }
	offset rawtell() { return file ? file->tell() : offset(-1); }

	offset size()
	{
		if (!file) return -1;
		offset pos = file->tell();
		if (!file->seek(-4, SEEK_END)) return -1;
		uint isize = file->getlil<uint>();
		return file->seek(pos, SEEK_SET) ? isize : offset(-1);
	}

	offset rawsize() { return file ? file->size() : offset(-1); }

	bool seek(offset pos, int whence)
	{
		if (writing || !reading) return false;

		if (whence == SEEK_END)
		{
			uchar skip[512];
			while (read(skip, sizeof(skip)) == sizeof(skip));
			return !pos;
		}
		else if (whence == SEEK_CUR) pos += zfile.total_out;

		if (pos >= (offset)zfile.total_out) pos -= zfile.total_out;
		else if (pos < 0 || !file->seek(headersize, SEEK_SET)) return false;
		else
		{
			if (zfile.next_in && zfile.total_in <= uint(zfile.next_in - buf))
			{
				zfile.avail_in += zfile.total_in;
				zfile.next_in -= zfile.total_in;
			}
			else
			{
				zfile.avail_in = 0;
				zfile.next_in = NULL;
			}
			inflateReset(&zfile);
			crc = crc32(0, NULL, 0);
		}

		uchar skip[512];
		while (pos > 0)
		{
			size_t skipped = (size_t)std::min(pos, (offset)sizeof(skip));
			if (read(skip, skipped) != skipped) { stopreading(); return false; }
			pos -= skipped;
		}

		return true;
	}

	size_t read(void* buf, size_t len)
	{
		if (!reading || !buf || !len) return 0;
		zfile.next_out = (Bytef*)buf;
		zfile.avail_out = len;
		while (zfile.avail_out > 0)
		{
			if (!zfile.avail_in)
			{
				readbuf(BUFSIZE);
				if (!zfile.avail_in) { stopreading(); break; }
			}
			int err = inflate(&zfile, Z_NO_FLUSH);
			if (err == Z_STREAM_END) { crc = crc32(crc, (Bytef*)buf, len - zfile.avail_out); finishreading(); stopreading(); return len - zfile.avail_out; }
			else if (err != Z_OK) { stopreading(); break; }
		}
		crc = crc32(crc, (Bytef*)buf, len - zfile.avail_out);
		return len - zfile.avail_out;
	}

	bool flushbuf(bool full = false)
	{
		if (full) deflate(&zfile, Z_SYNC_FLUSH);
		if (zfile.next_out && zfile.avail_out < BUFSIZE)
		{
			if (file->write(buf, BUFSIZE - zfile.avail_out) != BUFSIZE - zfile.avail_out || (full && !file->flush()))
				return false;
		}
		zfile.next_out = buf;
		zfile.avail_out = BUFSIZE;
		return true;
	}

	bool flush() { return flushbuf(true); }

	size_t write(const void* buf, size_t len)
	{
		if (!writing || !buf || !len) return 0;
		zfile.next_in = (Bytef*)buf;
		zfile.avail_in = len;
		while (zfile.avail_in > 0)
		{
			if (!zfile.avail_out && !flushbuf()) { stopwriting(); break; }
			int err = deflate(&zfile, Z_NO_FLUSH);
			if (err != Z_OK) { stopwriting(); break; }
		}
		crc = crc32(crc, (Bytef*)buf, len - zfile.avail_in);
		return len - zfile.avail_in;
	}
};

struct utf8stream : stream
{
	enum
	{
		BUFSIZE = 4096
	};
	stream* file;
	offset pos;
	size_t bufread, bufcarry, buflen;
	bool reading, writing, autoclose;
	uchar buf[BUFSIZE];

	utf8stream() : file(NULL), pos(0), bufread(0), bufcarry(0), buflen(0), reading(false), writing(false), autoclose(false)
	{
	}

	~utf8stream()
	{
		close();
	}

	bool readbuf(size_t size = BUFSIZE)
	{
		if (bufread >= bufcarry) { if (bufcarry > 0 && bufcarry < buflen) memmove(buf, &buf[bufcarry], buflen - bufcarry); buflen -= bufcarry; bufread = bufcarry = 0; }
		size_t n = file->read(&buf[buflen], std::min(size, BUFSIZE - buflen));
		if (n <= 0) return false;
		buflen += n;
		size_t carry = bufcarry;
		bufcarry += decodeutf8(&buf[bufcarry], BUFSIZE - bufcarry, &buf[bufcarry], buflen - bufcarry, &carry);
		if (carry > bufcarry && carry < buflen) { memmove(&buf[bufcarry], &buf[carry], buflen - carry); buflen -= carry - bufcarry; }
		return true;
	}

	bool checkheader()
	{
		size_t n = file->read(buf, 3);
		if (n == 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) return true;
		buflen = n;
		return false;
	}

	bool open(stream* f, const char* mode, bool needclose)
	{
		if (file) return false;
		for (; *mode; mode++)
		{
			if (*mode == 'r') { reading = true; break; }
			else if (*mode == 'w') { writing = true; break; }
		}
		if (!reading && !writing) return false;

		file = f;

		if (reading) checkheader();

		autoclose = needclose;
		return true;
	}

	void finishreading()
	{
		if (!reading) return;
	}

	void stopreading()
	{
		if (!reading) return;
		reading = false;
	}

	void stopwriting()
	{
		if (!writing) return;
		writing = false;
	}

	void close()
	{
		stopreading();
		stopwriting();
		if (autoclose) DELETEP(file);
	}

	bool end() { return !reading && !writing; }
	offset tell() { return reading || writing ? pos : offset(-1); }

	bool seek(offset off, int whence)
	{
		if (writing || !reading) return false;

		if (whence == SEEK_END)
		{
			uchar skip[512];
			while (read(skip, sizeof(skip)) == sizeof(skip));
			return !off;
		}
		else if (whence == SEEK_CUR) off += pos;

		if (off >= pos) off -= pos;
		else if (off < 0 || !file->seek(0, SEEK_SET)) return false;
		else
		{
			bufread = bufcarry = buflen = 0;
			pos = 0;
			checkheader();
		}

		uchar skip[512];
		while (off > 0)
		{
			size_t skipped = (size_t)std::min(off, (offset)sizeof(skip));
			if (read(skip, skipped) != skipped) { stopreading(); return false; }
			off -= skipped;
		}

		return true;
	}

	size_t read(void* dst, size_t len)
	{
		if (!reading || !dst || !len) return 0;
		size_t next = 0;
		while (next < len)
		{
			if (bufread >= bufcarry) { if (readbuf(BUFSIZE)) continue; stopreading(); break; }
			size_t n = std::min(len - next, bufcarry - bufread);
			memcpy(&((uchar*)dst)[next], &buf[bufread], n);
			next += n;
			bufread += n;
		}
		pos += next;
		return next;
	}

	bool getline(char* dst, size_t len)
	{
		if (!reading || !dst || !len) return false;
		--len;
		size_t next = 0;
		while (next < len)
		{
			if (bufread >= bufcarry) { if (readbuf(BUFSIZE)) continue; stopreading(); if (!next) return false; break; }
			size_t n = std::min(len - next, bufcarry - bufread);
			uchar* endline = (uchar*)memchr(&buf[bufread], '\n', n);
			if (endline) { n = endline + 1 - &buf[bufread]; len = next + n; }
			memcpy(&((uchar*)dst)[next], &buf[bufread], n);
			next += n;
			bufread += n;
		}
		dst[next] = '\0';
		pos += next;
		return true;
	}

	size_t write(const void* src, size_t len)
	{
		if (!writing || !src || !len) return 0;
		uchar dst[512];
		size_t next = 0;
		while (next < len)
		{
			size_t carry = 0, n = encodeutf8(dst, sizeof(dst), &((uchar*)src)[next], len - next, &carry);
			if (n > 0 && file->write(dst, n) != n) { stopwriting(); break; }
			next += carry;
		}
		pos += next;
		return next;
	}

	bool flush() { return file->flush(); }
};

stream* openrawfile(const char* filename, const char* mode)
{
	const char* found = findfile(filename, mode);
	if (!found) return NULL;
	filestream* file = new filestream;
	if (!file->open(found, mode)) { delete file; return NULL; }
	return file;
}

stream* openfile(const char* filename, const char* mode)
{
	stream* s = openzipfile(filename, mode);
	if (s) return s;
	return openrawfile(filename, mode);
}

stream* opentempfile(const char* name, const char* mode)
{
	const char* found = findfile(name, mode);
	filestream* file = new filestream;
	if (!file->opentemp(found ? found : name, mode)) { delete file; return NULL; }
	return file;
}

stream* opengzfile(const char* filename, const char* mode, stream* file, int level)
{
	stream* source = file ? file : openfile(filename, mode);
	if (!source) return NULL;
	gzstream* gz = new gzstream;
	if (!gz->open(source, mode, !file, level)) { if (!file) delete source; delete gz; return NULL; }
	return gz;
}

stream* openutf8file(const char* filename, const char* mode, stream* file)
{
	stream* source = file ? file : openfile(filename, mode);
	if (!source) return NULL;
	utf8stream* utf8 = new utf8stream;
	if (!utf8->open(source, mode, !file)) { if (!file) delete source; delete utf8; return NULL; }
	return utf8;
}