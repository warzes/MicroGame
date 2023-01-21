#pragma once

//=============================================================================
// Windows func
//=============================================================================

typedef void (*WindowResizeCallback)(int width, int height);
#if defined(_WIN32) || defined(__linux__)
typedef void (*WindowMaximizeCallback)(bool maximize);
#endif
typedef void (*WindowMinimizeCallback)(bool minimized);
typedef void (*WindowFocusCallback)(bool focused);

typedef void (*KeyCallback)(int key, int mod, bool isPressed);
typedef void (*CharCallback)(unsigned int key);
typedef void (*MouseButtonCallback)(int button, int action);
typedef void (*MouseMoveCallback)(int x, int y);
typedef void (*MouseScrollCallback)(double xoffset, double yoffset);
typedef void (*MouseCursorEnterCallback)(bool enter);

struct WindowCreateInfo
{
	const char* Title = "Game";
	int Width = 1024;
	int Height = 768;

	bool Fullscreen = false;
	bool Resizable = true;
	bool Vsync = false;

	// callback
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
};

bool CreateWindowSystem(const WindowCreateInfo& createInfo);
void DestroyWindowSystem();
void UpdateWindow();
void UpdateInput();
bool WindowShouldClose();

void SetWindowTitle(const char* title);
void SetWindowPosition(int posX, int posY);
void SetWindowSize(int width, int height);

int GetRenderWidth();
int GetRenderHeight();
float GetRenderAspectRatio();

bool IsWindowFullscreen(); // Check if window is currently fullscreen

// Monitor Info
int GetMonitorCount();                     // Get number of connected monitors
int GetCurrentMonitor();                   // Get current connected monitor
glm::vec2 GetMonitorPosition(int monitor); // Get specified monitor position
int GetMonitorWidth(int monitor);          // Get specified monitor width (current video mode used by monitor)
int GetMonitorHeight(int monitor);         // Get specified monitor height (current video mode used by monitor)
int GetMonitorPhysicalWidth(int monitor);  // Get specified monitor physical width in millimetres
int GetMonitorPhysicalHeight(int monitor); // Get specified monitor physical height in millimetres
int GetMonitorRefreshRate(int monitor);    // Get specified monitor refresh rate
const char* GetMonitorName(int monitor);   // Get the human-readable, UTF-8 encoded name of the primary monitor