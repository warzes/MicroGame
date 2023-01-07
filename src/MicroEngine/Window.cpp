#include "stdafx.h"
#include "BaseHeader.h"
#include "Window.h"
#include "Base.h"
#include "Core.h"
#include "Input.h"
//-------------------------------------------------------------------------
GLFWwindow* window = nullptr;
int FrameBufferWidth = 0;
int FrameBufferHeight = 0;
float FrameBufferAspectRatio = 0.0f;
bool Fullscreen = false;
bool WindowMaximized = false;
bool WindowMinimized = false;
bool WindowFocus = true;
//-----------------------------------------------------------------------------
WindowResizeCallback WindowResizeEvent = nullptr;
#if defined(_WIN32) || defined(__linux__)
WindowMaximizeCallback WindowMaximizeEvent = nullptr;
#endif
WindowMaximizeCallback WindowMinimizeEvent = nullptr;
WindowFocusCallback WindowFocusEvent = nullptr;
KeyCallback KeyEvent = nullptr;
CharCallback CharEvent = nullptr;
MouseButtonCallback MouseButtonEvent = nullptr;
MouseMoveCallback MouseMoveEvent = nullptr;
MouseScrollCallback MouseScrollEvent = nullptr;
MouseCursorEnterCallback MouseCursorEnterEvent = nullptr;
//-----------------------------------------------------------------------------
void errorCallback(int error, const char* description) noexcept
{
	LogError("GLFW Error " + std::to_string(error) + ": " + std::string(description));
}
//-----------------------------------------------------------------------------
void windowSizeCallback(GLFWwindow* /*window*/, int width, int height) noexcept
{
	FrameBufferWidth = Max(width, 1);
	FrameBufferHeight = Max(height, 1);

	if (FrameBufferHeight < 1) FrameBufferHeight = 1;
	FrameBufferAspectRatio = static_cast<float>(FrameBufferWidth) / static_cast<float>(FrameBufferHeight);
}
//-----------------------------------------------------------------------------
void windowSizeCallbackUser(GLFWwindow* window, int width, int height) noexcept
{
	windowSizeCallback(window, width, height);
	WindowResizeEvent(width, height);
}
//-----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__linux__)
void windowMaximizeCallback(GLFWwindow* /*window*/, int maximized) noexcept
{
	WindowMaximized = maximized == GL_TRUE;
}
#endif
//-----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__linux__)
void windowMaximizeCallbackUser(GLFWwindow* window, int maximized) noexcept
{
	windowMaximizeCallback(window, maximized);
	WindowMaximizeEvent(maximized == GL_TRUE);
}
#endif
//-----------------------------------------------------------------------------
void windowMinimizeCallback(GLFWwindow* /*window*/, int minimized) noexcept
{
	WindowMinimized = minimized == GL_TRUE;
}
//-----------------------------------------------------------------------------
void windowMinimizeCallbackUser(GLFWwindow* window, int minimized) noexcept
{
	windowMinimizeCallback(window, minimized);
	WindowMinimizeEvent(minimized == GL_TRUE);
}
//-----------------------------------------------------------------------------
void windowFocusCallback(GLFWwindow* /*window*/, int focused) noexcept
{
	WindowFocus = focused == GL_TRUE;
}
//-----------------------------------------------------------------------------
void windowFocusCallbackUser(GLFWwindow* window, int focused) noexcept
{
	windowFocusCallback(window, focused);
	WindowFocusEvent(focused == GL_TRUE);
}
//-----------------------------------------------------------------------------
bool CreateWindowSystem(const WindowCreateInfo& createInfo)
{
	assert(!window);

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit())
	{
		LogError("GLFW: Failed to initialize GLFW");
		return false;
	}
	glfwDefaultWindowHints();  // Set default windows hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(_DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif

	glfwWindowHint(GLFW_RESIZABLE, createInfo.Resizable ? GL_TRUE : GL_FALSE);

	window = glfwCreateWindow(createInfo.Width, createInfo.Height, createInfo.Title, NULL, NULL);
	if (!window)
	{
		LogError("GLFW: Failed to initialize Window");
		return false;
	}

	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);
	if (FrameBufferWidth == 0 || FrameBufferHeight == 0)
		return false;
	FrameBufferAspectRatio = static_cast<float>(FrameBufferWidth) / static_cast<float>(FrameBufferHeight);

	WindowResizeEvent = createInfo.WindowResizeEvent;
	WindowMaximizeEvent = createInfo.WindowMaximizeEvent;
	WindowMinimizeEvent = createInfo.WindowMinimizeEvent;
	WindowFocusEvent = createInfo.WindowFocusEvent;
	KeyEvent = createInfo.KeyEvent;
	CharEvent = createInfo.CharEvent;
	MouseButtonEvent = createInfo.MouseButtonEvent;
	MouseMoveEvent = createInfo.MouseMoveEvent;
	MouseScrollEvent = createInfo.MouseScrollEvent;

	glfwSetWindowSizeCallback(window, WindowResizeEvent ? windowSizeCallbackUser : windowSizeCallback);
#if !defined(PLATFORM_WEB)
	glfwSetWindowMaximizeCallback(window, WindowMaximizeEvent ? windowMaximizeCallbackUser : windowMaximizeCallback);
#endif
	glfwSetWindowIconifyCallback(window, WindowMinimizeEvent ? windowMinimizeCallbackUser : windowMinimizeCallback);
	glfwSetWindowFocusCallback(window, WindowFocusEvent ? windowFocusCallbackUser : windowFocusCallback);

	extern void InitInput(GLFWwindow*);
	InitInput(window);
	
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress))
	{
		LogError("GLAD: Cannot load OpenGL extensions");
		return false;
	}

	glfwSwapInterval(createInfo.Vsync ? 1 : 0);	

	return true;
}
//-----------------------------------------------------------------------------
void DestroyWindowSystem()
{
	glfwDestroyWindow(window);
	window = nullptr;
	glfwTerminate();
}
//-----------------------------------------------------------------------------
bool WindowShouldClose()
{
	return glfwWindowShouldClose(window) == GLFW_TRUE;
}
//-----------------------------------------------------------------------------
void SetWindowTitle(const char* title)
{
	glfwSetWindowTitle(window, title);
}
//-----------------------------------------------------------------------------
void SetWindowPosition(int posX, int posY)
{
	glfwSetWindowPos(window, posX, posY);
}
//-----------------------------------------------------------------------------
void SetWindowSize(int width, int height)
{
	glfwSetWindowSize(window, width, height);
	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);
}
//-----------------------------------------------------------------------------
void UpdateWindow()
{
	glfwSwapBuffers(window);
}
//-----------------------------------------------------------------------------
int GetRenderWidth()
{
	return FrameBufferWidth;
}
//-----------------------------------------------------------------------------
int GetRenderHeight()
{
	return FrameBufferHeight;
}
//-----------------------------------------------------------------------------
float GetRenderAspectRatio()
{
	return FrameBufferAspectRatio;
}
//-----------------------------------------------------------------------------
bool IsWindowFullscreen()
{
	return Fullscreen;
}
//-----------------------------------------------------------------------------
int GetMonitorCount()
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	glfwGetMonitors(&monitorCount);
	return monitorCount;
#else
	return 1;
#endif
}
//-----------------------------------------------------------------------------
int GetCurrentMonitor()
{
	int index = 0;

#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	GLFWmonitor* monitor = NULL;

	if (monitorCount > 1)
	{
		if (IsWindowFullscreen())
		{
			// Get the handle of the monitor that the specified window is in full screen on
			monitor = glfwGetWindowMonitor(window);

			for (int i = 0; i < monitorCount; i++)
			{
				if (monitors[i] == monitor)
				{
					index = i;
					break;
				}
			}
		}
		else
		{
			int x = 0;
			int y = 0;

			glfwGetWindowPos(window, &x, &y);

			for (int i = 0; i < monitorCount; i++)
			{
				int mx = 0;
				int my = 0;

				int width = 0;
				int height = 0;

				monitor = monitors[i];
				glfwGetMonitorWorkarea(monitor, &mx, &my, &width, &height);

				if (x >= mx && x <= (mx + width) && y >= my && y <= (my + height))
				{
					index = i;
					break;
				}
			}
		}
	}
#endif

	return index;
}
//-----------------------------------------------------------------------------
glm::vec2 GetMonitorPosition(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		int x, y;
		glfwGetMonitorPos(monitors[monitor], &x, &y);

		return { (float)x, (float)y };
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return { 0.0f, 0.0f };
}
//-----------------------------------------------------------------------------
int GetMonitorWidth(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);

		if (mode) return mode->width;
		else LogWarning("GLFW: Failed to find video mode for selected monitor");
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return 0;
}
//-----------------------------------------------------------------------------
int GetMonitorHeight(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);

		if (mode) return mode->height;
		else LogWarning("GLFW: Failed to find video mode for selected monitor");
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return 0;
}
//-----------------------------------------------------------------------------
int GetMonitorPhysicalWidth(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		int physicalWidth;
		glfwGetMonitorPhysicalSize(monitors[monitor], &physicalWidth, NULL);
		return physicalWidth;
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return 0;
}
//-----------------------------------------------------------------------------
int GetMonitorPhysicalHeight(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		int physicalHeight;
		glfwGetMonitorPhysicalSize(monitors[monitor], NULL, &physicalHeight);
		return physicalHeight;
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return 0;
}
//-----------------------------------------------------------------------------
int GetMonitorRefreshRate(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		const GLFWvidmode* vidmode = glfwGetVideoMode(monitors[monitor]);
		return vidmode->refreshRate;
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return 0;
}
//-----------------------------------------------------------------------------
const char* GetMonitorName(int monitor)
{
#if defined(_WIN32) || defined(__linux__)
	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	if ((monitor >= 0) && (monitor < monitorCount))
	{
		return glfwGetMonitorName(monitors[monitor]);
	}
	else LogWarning("GLFW: Failed to find selected monitor");
#endif
	return "";
}
//-----------------------------------------------------------------------------