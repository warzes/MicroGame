#pragma once

#include "0_EngineConfig.h"
#include "1_BaseHeader.h"

#if defined(_MSC_VER)
#	define SE_FORCE_INLINE __forceinline
#	define SE_CONST_FUNC  __declspec(noalias)
#elif defined(__clang__) || defined(__GNUC__)
#	define SE_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define SE_CONST_FUNC  __attribute__( (pure) )
#endif

#define SE_CONSTEXPR_FUNC constexpr SE_CONST_FUNC

namespace base
{
	template<typename Ty>
	inline SE_CONSTEXPR_FUNC bool IsAligned(Ty _a, int32_t _align)
	{
		const Ty mask = Ty(_align - 1);
		return 0 == (_a & mask);
	}

	template<typename Ty>
	inline SE_CONSTEXPR_FUNC bool IsAligned(const Ty* _ptr, int32_t _align)
	{
		union { const void* ptr; uintptr_t addr; } un = { _ptr };
		return IsAligned(un.addr, _align);
	}

	inline constexpr int Min(int a, int b) { return a < b ? a : b; }
	inline constexpr float Min(float a, float b) { return a < b ? a : b; }
	//template<typename T> inline constexpr T Min(const T& a, const T& b) { return a < b ? a : b; }

	inline constexpr int Max(int a, int b) { return a > b ? a : b; }
	inline constexpr float Max(float a, float b) { return a > b ? a : b; }
	//template<typename T> inline constexpr T Max(const T& a, const T& b) { return a > b ? a : b; }

	inline constexpr int Clamp(int a, int min, int max) { return Max(Min(a, max), min); }
	inline constexpr float Clamp(float a, float min, float max) { return Max(Min(a, max), min); }
	//template<typename T> inline constexpr T Clamp(const T& a, const T& min, const T& max) { return Max(Min(a, max), min); }

	inline constexpr float Lerp(float lower, float upper, float gradient)
	{
		return lower + (upper - lower) * Max(0.0f, Min(gradient, 1.0f));
	}

	inline constexpr glm::vec3 Mix(const glm::vec3& a, const glm::vec3& b, float t)
	{
		return (a * (1 - t)) + (b * (t));
	}

	// Implementation from "08/02/2015 Better array 'countof' implementation with C++ 11 (updated)" - https://www.g-truc.net/post-0708.html
	template<typename T, size_t N>
	[[nodiscard]] constexpr size_t Countof(T const (&)[N])
	{
		return N;
	}
}