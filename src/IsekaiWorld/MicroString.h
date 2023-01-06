#pragma once

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#define _vsnprintf vsnprintf
#endif

#define MAXSTRLEN 260
typedef char string[MAXSTRLEN];

inline void vformatstring(char* d, const char* fmt, va_list v, int len) { _vsnprintf(d, len, fmt, v); d[len - 1] = 0; }
template<size_t N> inline void vformatstring(char(&d)[N], const char* fmt, va_list v) { vformatstring(d, fmt, v, N); }

inline char* copystring(char* d, const char* s, size_t len)
{
	size_t slen = std::min(strlen(s), len - 1);
	memcpy(d, s, slen);
	d[slen] = 0;
	return d;
}
template<size_t N> inline char* copystring(char(&d)[N], const char* s) { return copystring(d, s, N); }

inline char* concatstring(char* d, const char* s, size_t len) { size_t used = strlen(d); return used < len ? copystring(d + used, s, len - used) : d; }
template<size_t N> inline char* concatstring(char(&d)[N], const char* s) { return concatstring(d, s, N); }

inline char* prependstring(char* d, const char* s, size_t len)
{
	size_t slen = std::min(strlen(s), len);
	memmove(&d[slen], d, std::min(len - slen, strlen(d) + 1));
	memcpy(d, s, slen);
	d[len - 1] = 0;
	return d;
}
template<size_t N> inline char* prependstring(char(&d)[N], const char* s) { return prependstring(d, s, N); }

inline void nformatstring(char* d, int len, const char* fmt, ...);
inline void nformatstring(char* d, int len, const char* fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	vformatstring(d, fmt, v, len);
	va_end(v);
}

template<size_t N> inline void formatstring(char(&d)[N], const char* fmt, ...);
template<size_t N> inline void formatstring(char(&d)[N], const char* fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	vformatstring(d, fmt, v, int(N));
	va_end(v);
}

template<size_t N> inline void concformatstring(char(&d)[N], const char* fmt, ...);
template<size_t N> inline void concformatstring(char(&d)[N], const char* fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	int len = strlen(d);
	vformatstring(d + len, fmt, v, int(N) - len);
	va_end(v);
}

#define defformatstring(d,...) string d; formatstring(d, __VA_ARGS__)
#define defvformatstring(d,last,fmt) string d; { va_list ap; va_start(ap, last); vformatstring(d, fmt, ap); va_end(ap); }

template<size_t N> inline bool matchstring(const char* s, size_t len, const char(&d)[N])
{
	return len == N - 1 && !memcmp(s, d, N - 1);
}

inline bool matchstring(const char* s, size_t len, const char* d, size_t len2)
{
	return len == len2 && !memcmp(s, d, len);
}

inline char* newstring(size_t l) { return new char[l + 1]; }
inline char* newstring(const char* s, size_t l) { return copystring(newstring(l), s, l + 1); }
inline char* newstring(const char* s) { size_t l = strlen(s); char* d = newstring(l); memcpy(d, s, l + 1); return d; }

struct stringslice
{
	const char* str;
	int len;
	stringslice() {}
	stringslice(const char* str, int len) : str(str), len(len) {}
	stringslice(const char* str, const char* end) : str(str), len(int(end - str)) {}

	const char* end() const { return &str[len]; }
};

inline char* newstring(const stringslice& s) { return newstring(s.str, s.len); }
inline const char* stringptr(const char* s) { return s; }
inline const char* stringptr(const stringslice& s) { return s.str; }
inline int stringlen(const char* s) { return int(strlen(s)); }
inline int stringlen(const stringslice& s) { return s.len; }

inline char* copystring(char* d, const stringslice& s, size_t len)
{
	size_t slen = std::min(size_t(s.len), len - 1);
	memcpy(d, s.str, slen);
	d[slen] = 0;
	return d;
}
template<size_t N> inline char* copystring(char(&d)[N], const stringslice& s) { return copystring(d, s, N); }

inline uint32_t memhash(const void* ptr, int len)
{
	const uint8_t* data = (const uint8_t*)ptr;
	uint32_t h = 5381;

	for (int i = 0; i < len; i++)
		h = ((h << 5) + h) ^ data[i];
	return h;
}

inline uint32_t HTHash(const stringslice& s) { return memhash(s.str, s.len); }

static inline bool HTCmp(const stringslice& x, const char* y)
{
	return x.len == (int)strlen(y) && !memcmp(x.str, y, x.len);
}