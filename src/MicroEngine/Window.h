#pragma once

//=============================================================================
// Windows func
//=============================================================================

typedef void (*WindowResizeCallback)(int width, int height);
typedef void (*MouseButtonCallback)(int button, int action);

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
	MouseButtonCallback MouseButtonEvent = nullptr;
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