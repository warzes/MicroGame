#define _CRT_SECURE_NO_WARNINGS
#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4255)
#	pragma warning(disable : 4706)
#	pragma warning(disable : 4820)
#	pragma warning(disable : 5045)
#endif

#if defined(_WIN32)
#define _GLFW_WIN32
#endif
#if defined(__linux__)
#if !defined(_GLFW_WAYLAND)     // Required for Wayland windowing
#define _GLFW_X11
#endif
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#define _GLFW_X11
#endif
#if defined(__APPLE__)
#define _GLFW_COCOA
#define _GLFW_USE_MENUBAR       // To create and populate the menu bar when the first window is created
#define _GLFW_USE_RETINA        // To have windows use the full resolution of Retina displays
#endif
#if defined(__TINYC__)
#define _WIN32_WINNT_WINXP      0x0501
#endif

// Common modules to all platforms
#include <GLFW/context.c>
#include <GLFW/init.c>
#include <GLFW/input.c>
#include <GLFW/monitor.c>
#include <GLFW/vulkan.c>
#include <GLFW/window.c>
#include <GLFW/platform.c>

#if defined(_WIN32)
#include <GLFW/win32_init.c>
#include <GLFW/win32_module.c>
#include <GLFW/win32_joystick.c>
#include <GLFW/win32_monitor.c>
#include <GLFW/win32_time.c>
#include <GLFW/win32_thread.c>
#include <GLFW/win32_window.c>
#include <GLFW/wgl_context.c>
#include <GLFW/egl_context.c>
#include <GLFW/osmesa_context.c>
#endif

#if defined(__linux__)
#if defined(_GLFW_WAYLAND)
#include <GLFW/wl_init.c>
#include <GLFW/wl_monitor.c>
#include <GLFW/wl_window.c>
#include <GLFW/wayland-pointer-constraints-unstable-v1-client-protocol.c>
#include <GLFW/wayland-relative-pointer-unstable-v1-client-protocol.c>
#endif
#if defined(_GLFW_X11)
#include <GLFW/x11_init.c>
#include <GLFW/x11_monitor.c>
#include <GLFW/x11_window.c>
#include <GLFW/glx_context.c>
#endif

#include <GLFW/linux_joystick.c>
#include <GLFW/posix_thread.c>
#include <GLFW/posix_time.c>
#include <GLFW/xkb_unicode.c>
#include <GLFW/egl_context.c>
#include <GLFW/osmesa_context.c>
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined( __NetBSD__) || defined(__DragonFly__)
#include <GLFW/x11_init.c>
#include <GLFW/x11_monitor.c>
#include <GLFW/x11_window.c>
#include <GLFW/xkb_unicode.c>
#include <GLFW/null_joystick.c>
#include <GLFW/posix_time.c>
#include <GLFW/posix_thread.c>
#include <GLFW/glx_context.c>
#include <GLFW/egl_context.c>
#include <GLFW/osmesa_context.c>
#endif

#if defined(__APPLE__)
#include <GLFW/cocoa_init.m>
#include <GLFW/cocoa_joystick.m>
#include <GLFW/cocoa_monitor.m>
#include <GLFW/cocoa_window.m>
#include <GLFW/cocoa_time.c>
#include <GLFW/posix_thread.c>
#include <GLFW/nsgl_context.m>
#include <GLFW/egl_context.c>
#include <GLFW/osmesa_context.c>
#endif

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
#undef _CRT_SECURE_NO_WARNINGS