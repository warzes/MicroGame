#pragma once

#include "TempBase.h"

#define SDL_LIL_ENDIAN  1234
#define SDL_BIG_ENDIAN  4321

#ifndef SDL_BYTEORDER
#ifdef __linux__
#include <endian.h>
#define SDL_BYTEORDER  __BYTE_ORDER
#elif defined(__OpenBSD__) || defined(__DragonFly__)
#include <endian.h>
#define SDL_BYTEORDER  BYTE_ORDER
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#define SDL_BYTEORDER  BYTE_ORDER
/* predefs from newer gcc and clang versions: */
#elif defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__BYTE_ORDER__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define SDL_BYTEORDER   SDL_LIL_ENDIAN
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define SDL_BYTEORDER   SDL_BIG_ENDIAN
#else
#error Unsupported endianness
#endif /**/
#else
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__) || \
    defined(__sparc__)
#define SDL_BYTEORDER   SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER   SDL_LIL_ENDIAN
#endif
#endif /* __linux__ */
#endif /* !SDL_BYTEORDER */

/* various modern compilers may have builtin swap */
#if defined(__GNUC__) || defined(__clang__)
#   define HAS_BUILTIN_BSWAP16 (SDL_HAS_BUILTIN(__builtin_bswap16)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#   define HAS_BUILTIN_BSWAP32 (SDL_HAS_BUILTIN(__builtin_bswap32)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
#   define HAS_BUILTIN_BSWAP64 (SDL_HAS_BUILTIN(__builtin_bswap64)) || \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))

    /* this one is broken */
#   define HAS_BROKEN_BSWAP (__GNUC__ == 2 && __GNUC_MINOR__ <= 95)
#else
#   define HAS_BUILTIN_BSWAP16 0
#   define HAS_BUILTIN_BSWAP32 0
#   define HAS_BUILTIN_BSWAP64 0
#   define HAS_BROKEN_BSWAP 0
#endif

#if HAS_BUILTIN_BSWAP16
#define SDL_Swap16(x) __builtin_bswap16(x)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma intrinsic(_byteswap_ushort)
#define SDL_Swap16(x) _byteswap_ushort(x)
#elif defined(__i386__) && !HAS_BROKEN_BSWAP
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    __asm__("xchgb %b0,%h0": "=q"(x) : "0"(x));
    return x;
}
#elif defined(__x86_64__)
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    __asm__("xchgb %b0,%h0": "=Q"(x) : "0"(x));
    return x;
}
#elif (defined(__powerpc__) || defined(__ppc__))
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    int result;

    __asm__("rlwimi %0,%2,8,16,23": "=&r"(result) : "0"(x >> 8), "r"(x));
    return (Uint16)result;
}
#elif (defined(__m68k__) && !defined(__mcoldfire__))
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    __asm__("rorw #8,%0": "=d"(x) : "0"(x) : "cc");
    return x;
}
#elif defined(__WATCOMC__) && defined(__386__)
extern __inline Uint16 SDL_Swap16(Uint16);
#pragma aux SDL_Swap16 = \
  "xchg al, ah" \
  parm   [ax]   \
  modify [ax];
#else
SDL_FORCE_INLINE Uint16
SDL_Swap16(Uint16 x)
{
    return SDL_static_cast(Uint16, ((x << 8) | (x >> 8)));
}
#endif

#if HAS_BUILTIN_BSWAP32
#define SDL_Swap32(x) __builtin_bswap32(x)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma intrinsic(_byteswap_ulong)
#define SDL_Swap32(x) _byteswap_ulong(x)
#elif defined(__i386__) && !HAS_BROKEN_BSWAP
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    __asm__("bswap %0": "=r"(x) : "0"(x));
    return x;
}
#elif defined(__x86_64__)
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    __asm__("bswapl %0": "=r"(x) : "0"(x));
    return x;
}
#elif (defined(__powerpc__) || defined(__ppc__))
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    Uint32 result;

    __asm__("rlwimi %0,%2,24,16,23": "=&r"(result) : "0" (x >> 24), "r"(x));
    __asm__("rlwimi %0,%2,8,8,15"  : "=&r"(result) : "0" (result), "r"(x));
    __asm__("rlwimi %0,%2,24,0,7"  : "=&r"(result) : "0" (result), "r"(x));
    return result;
}
#elif (defined(__m68k__) && !defined(__mcoldfire__))
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    __asm__("rorw #8,%0\n\tswap %0\n\trorw #8,%0": "=d"(x) : "0"(x) : "cc");
    return x;
}
#elif defined(__WATCOMC__) && defined(__386__)
extern __inline Uint32 SDL_Swap32(Uint32);
#pragma aux SDL_Swap32 = \
  "bswap eax"  \
  parm   [eax] \
  modify [eax];
#else
SDL_FORCE_INLINE Uint32
SDL_Swap32(Uint32 x)
{
    return SDL_static_cast(Uint32, ((x << 24) | ((x << 8) & 0x00FF0000) |
        ((x >> 8) & 0x0000FF00) | (x >> 24)));
}
#endif

#if HAS_BUILTIN_BSWAP64
#define SDL_Swap64(x) __builtin_bswap64(x)
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma intrinsic(_byteswap_uint64)
#define SDL_Swap64(x) _byteswap_uint64(x)
#elif defined(__i386__) && !HAS_BROKEN_BSWAP
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
    union {
        struct {
            Uint32 a, b;
        } s;
        Uint64 u;
    } v;
    v.u = x;
    __asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1"
        : "=r"(v.s.a), "=r"(v.s.b)
        : "0" (v.s.a), "1"(v.s.b));
    return v.u;
}
#elif defined(__x86_64__)
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
    __asm__("bswapq %0": "=r"(x) : "0"(x));
    return x;
}
#elif defined(__WATCOMC__) && defined(__386__)
extern __inline Uint64 SDL_Swap64(Uint64);
#pragma aux SDL_Swap64 = \
  "bswap eax"     \
  "bswap edx"     \
  "xchg eax,edx"  \
  parm [eax edx]  \
  modify [eax edx];
#else
SDL_FORCE_INLINE Uint64
SDL_Swap64(Uint64 x)
{
    Uint32 hi, lo;

    /* Separate into high and low 32-bit values and swap them */
    lo = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
    x >>= 32;
    hi = SDL_static_cast(Uint32, x & 0xFFFFFFFF);
    x = SDL_Swap32(lo);
    x <<= 32;
    x |= SDL_Swap32(hi);
    return (x);
}
#endif

const int islittleendian = 1;
#ifdef SDL_BYTEORDER
#define endianswap16 SDL_Swap16
#define endianswap32 SDL_Swap32
#define endianswap64 SDL_Swap64
#else
inline ushort endianswap16(ushort n) { return (n << 8) | (n >> 8); }
inline uint endianswap32(uint n) { return (n << 24) | (n >> 24) | ((n >> 8) & 0xFF00) | ((n << 8) & 0xFF0000); }
inline ullong endianswap64(ullong n) { return endianswap32(uint(n >> 32)) | ((ullong)endianswap32(uint(n)) << 32); }
#endif
template<class T> inline T endianswap(T n) { union { T t; uint i; } conv; conv.t = n; conv.i = endianswap32(conv.i); return conv.t; }
template<> inline ushort endianswap<ushort>(ushort n) { return endianswap16(n); }
template<> inline short endianswap<short>(short n) { return endianswap16(n); }
template<> inline uint endianswap<uint>(uint n) { return endianswap32(n); }
template<> inline int endianswap<int>(int n) { return endianswap32(n); }
template<> inline ullong endianswap<ullong>(ullong n) { return endianswap64(n); }
template<> inline llong endianswap<llong>(llong n) { return endianswap64(n); }
template<> inline double endianswap<double>(double n) { union { double t; uint i; } conv; conv.t = n; conv.i = endianswap64(conv.i); return conv.t; }
template<class T> inline void endianswap(T* buf, size_t len) { for (T* end = &buf[len]; buf < end; buf++) *buf = endianswap(*buf); }
template<class T> inline T endiansame(T n) { return n; }
template<class T> inline void endiansame(T* buf, size_t len) {}
#ifdef SDL_BYTEORDER
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define lilswap endiansame
#define bigswap endianswap
#else
#define lilswap endianswap
#define bigswap endiansame
#endif
#elif defined(__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lilswap endiansame
#define bigswap endianswap
#else
#define lilswap endianswap
#define bigswap endiansame
#endif
#else
template<class T> inline T lilswap(T n) { return *(const uchar*)&islittleendian ? n : endianswap(n); }
template<class T> inline void lilswap(T* buf, size_t len) { if (!*(const uchar*)&islittleendian) endianswap(buf, len); }
template<class T> inline T bigswap(T n) { return *(const uchar*)&islittleendian ? endianswap(n) : n; }
template<class T> inline void bigswap(T* buf, size_t len) { if (*(const uchar*)&islittleendian) endianswap(buf, len); }
#endif