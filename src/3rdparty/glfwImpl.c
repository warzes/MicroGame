#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4255)
#	pragma warning(disable : 4706)
#	pragma warning(disable : 4820)
#	pragma warning(disable : 5045)
#	define _CRT_SECURE_NO_WARNINGS
#endif

// Tool update in https://github.com/warzes/glfw-single-header

#define GLFW_INCLUDE_NONE
#define GLFW_NATIVE_INCLUDE_NONE
#define _GLFW_IMPLEMENTATION
#include <glfwSingleFile.h>
#undef _GLFW_IMPLEMENTATION


#if defined(_WIN32)
#ifdef __cplusplus
extern "C" {
#endif
    void* glfwGetWin32WindowTemp(GLFWwindow* handle)
    {
        _GLFWwindow* window = (_GLFWwindow*)handle;
        _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

        if( _glfw.platform.platformID != GLFW_PLATFORM_WIN32 )
        {
            _glfwInputError(GLFW_PLATFORM_UNAVAILABLE,
                "Win32: Platform not initialized");
            return NULL;
        }

        return window->win32.handle;
    }
#ifdef __cplusplus
}
#endif
#endif

#if defined(_MSC_VER)
#	undef _CRT_SECURE_NO_WARNINGS
#	pragma warning(pop)
#endif