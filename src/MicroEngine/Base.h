#pragma once

#include "BaseHeader.h"

//=============================================================================
// Preprocessor
//=============================================================================

// Turn argument to string constant:
// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html#Stringizing
#define SE_STRINGIZE( _n )   SE_STRINGIZE_2( _n )
#define SE_STRINGIZE_2( _n ) #_n

#if defined(_MSC_VER)
#	define TODO( _msg )  __pragma(message("" __FILE__ "(" SE_STRINGIZE(__LINE__) "): TODO: " _msg))
#else
#	define TODO( _msg )
#endif

#if defined(_MSC_VER)
#	define SE_FORCE_INLINE __forceinline
#	define SE_CONST_FUNC  __declspec(noalias)
#	define SE_NO_VTABLE   __declspec(novtable) // https://habr.com/ru/post/442340/
#elif defined(__clang__) || defined(__GNUC__)
#	define SE_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define SE_CONST_FUNC  __attribute__( (pure) )
#	define SE_NO_VTABLE
#endif

template <class T> inline void SE_UNUSED(const T&) {}

enum SE_EMPTY { SEEmpty };
enum SE_ZERO { SEZero };
enum SE_IDENTITY { SEIdentity };

//=============================================================================
// Basic Inline Func
//=============================================================================

template <typename T>
inline void SafeDelete(T*& resource)
{
	delete resource;
	resource = nullptr;
}

template <typename T>
inline void SafeDeleteArray(T*& resource)
{
	delete[] resource;
	resource = nullptr;
}

inline constexpr bool IsPowerOfTwo(uint32_t x)
{
	return x != 0 && (x & (x - 1)) == 0;
}

inline constexpr uint32_t NextPowerOfTwo(uint32_t x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

inline bool EqualsF(float a, float b, float eps)
{
	return (fabs(a - b) < eps);
}

template<class T>
inline constexpr void Swap(T& left, T& right) noexcept
{
	T tmp = std::move(left);
	left = std::move(right);
	right = std::move(tmp);
}

inline constexpr int Min(int a, int b) noexcept { return a < b ? a : b; }
inline constexpr float Min(float a, float b) noexcept { return a < b ? a : b; }
inline constexpr glm::vec3 Min(const glm::vec3& v1, const glm::vec3& v2)
{
	return { Min(v1.x, v2.x), Min(v1.y, v2.y), Min(v1.z, v2.z) };
}

inline constexpr int Max(int a, int b) noexcept { return a > b ? a : b; }
inline constexpr float Max(float a, float b) noexcept { return a > b ? a : b; }
inline constexpr glm::vec3 Max(const glm::vec3& v1, const glm::vec3& v2)
{
	return { Max(v1.x, v2.x), Max(v1.y, v2.y), Max(v1.z, v2.z) };
}

inline constexpr int Clamp(int a, int min, int max) noexcept { return Max(Min(a, max), min); }
inline constexpr float Clamp(float a, float min, float max) noexcept { return Max(Min(a, max), min); }

inline constexpr float Lerp(float lower, float upper, float gradient) noexcept
{
	return lower + (upper - lower) * Max(0.0f, Min(gradient, 1.0f));
}

inline constexpr glm::vec3 Mix(const glm::vec3& a, const glm::vec3& b, float t) noexcept
{
	return (a * (1 - t)) + (b * (t));
}

// Implementation from "08/02/2015 Better array 'countof' implementation with C++ 11 (updated)" - https://www.g-truc.net/post-0708.html
template<typename T, size_t N>
[[nodiscard]] constexpr size_t Countof(T const (&)[N])
{
	return N;
}