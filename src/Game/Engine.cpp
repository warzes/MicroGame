#include "stdafx.h"
#include "Engine.h"
//=============================================================================
#if defined(_MSC_VER)
//#	pragma comment( lib, "3rdparty.lib" )
#endif
//=============================================================================
#if defined(_WIN32)
extern "C"
{
	// NVIDIA: Force usage of NVidia GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> See "Enabling High Performance Graphics Rendering on Optimus Systems" http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

	// AMD: Force usage of AMD GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> Named "Dynamic Switchable Graphics", found no official documentation, only https://community.amd.com/message/1307599#comment-1307599 - "Can an OpenGL app default to the discrete GPU on an Enduro system?"
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // _WIN32
//=============================================================================
#pragma region Header

#if defined(_MSC_VER)
#	pragma warning(disable : 5045)
#	pragma warning(push, 0)
//#	pragma warning(disable : 4365)
#endif

#include <cassert>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <string>
#include <unordered_map>
#include <fstream>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_NONE

#include <stb/stb_image.h>
#include <stb/stb_truetype.h>

#include <tiny_obj_loader.h>

namespace std
{
	template <>
	struct hash<renderer::Vertex_Pos3_TexCoord>
	{
		size_t operator()(const renderer::Vertex_Pos3_TexCoord& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)) >> 1);
		}
	};
} // namespace std

#if defined(_WIN32) && defined(_DEBUG)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char*);
#endif

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif

#pragma endregion
//=============================================================================
#pragma region Base
namespace base
{
}
#pragma endregion
//=============================================================================
#pragma region Core
namespace core
{
	//-------------------------------------------------------------------------
	// Logging
	//-------------------------------------------------------------------------
	constexpr auto LogSeperator = "********************************************************************************************************";
#if defined(_WIN32) || defined(__linux__)
	FILE* logFile = nullptr;
#endif

	void logPrint(const char* simplePrefix, const char* clrPrefix, const char* str)
	{
#if defined(__ANDROID__)
		__android_log_write(ANDROID_LOG_INFO, "SE_APP", str);
#elif defined(__EMSCRIPTEN__)
		emscripten_log(EM_LOG_CONSOLE, "%s", str);
#else
#	if defined(_WIN32) && defined(_DEBUG)

		if (simplePrefix)
			OutputDebugStringA((std::string(simplePrefix) + str).c_str());
		else
			OutputDebugStringA(str);
		OutputDebugStringA("\n");
#	endif

		if (clrPrefix)
			puts((std::string(clrPrefix) + str).c_str());
		else
			puts(str);

		if (logFile)
		{
			if (simplePrefix)
				fputs((std::string(simplePrefix) + str).c_str(), logFile);
			else
				fputs(str, logFile);
			fputs("\n", logFile);
		}
#endif
		}

	bool CreateLogSystem(const core::LogCreateInfo& createInfo)
	{
#if defined(_WIN32)
		assert(!logFile);

		errno_t fileErr;
		fileErr = fopen_s(&logFile, createInfo.FileName, "w");
		if (fileErr != 0 || !logFile)
		{
			LogError("FileLog open failed!!!");
			logFile = nullptr;
		}
#endif
		
		const std::time_t rawtime = time(nullptr);
		char str[26] = { 0 };
		ctime_s(str, sizeof str, &rawtime);

		LogPrint(LogSeperator);
		LogPrint(std::string(str) + "Log Started.");
		LogPrint(LogSeperator);
		LogPrint("");

		return true;
	}

	void DestroyLogSystem()
	{
		const std::time_t rawtime = time(nullptr);
		char str[26] = { 0 };
		ctime_s(str, sizeof str, &rawtime);

		LogPrint("");
		LogPrint(LogSeperator);
		LogPrint(std::string(str) + "Log Ended.");
		LogPrint(LogSeperator);

#if defined(_WIN32) || defined(__linux__)
		if (logFile)
		{
			fclose(logFile);
			logFile = nullptr;
		}
#endif
	}

	void LogPrint(const char* str)
	{
		logPrint(nullptr, nullptr, str);
	}
		
	void LogWarning(const char* str)
	{
		logPrint("[ WARNING ] : ", "[ \033[33mWARNING\033[0m ] : ", str);
	}

	void LogError(const char* str)
	{
		logPrint("[ ERROR   ] : ", "[ \033[31mERROR\033[0m   ] : ", str);
	}
}
#pragma endregion
//=============================================================================
#pragma region Math
namespace math
{

}
#pragma endregion
//=============================================================================
#pragma region Platform
namespace platform
{
	//-------------------------------------------------------------------------
	// Input
	//-------------------------------------------------------------------------
	extern GLFWwindow* window;

	constexpr auto MAX_KEYBOARD_KEYS = 512;     // Maximum number of keyboard keys supported
	constexpr auto MAX_KEY_PRESSED_QUEUE = 16;  // Maximum number of keys in the key input queue
	constexpr auto MAX_CHAR_PRESSED_QUEUE = 16; // Maximum number of characters in the char input queue
	constexpr auto MAX_MOUSE_BUTTONS = 8;       // Maximum number of mouse buttons supported

	struct
	{
		char currentKeyState[MAX_KEYBOARD_KEYS] = { 0 };      // Registers current frame key state
		char previousKeyState[MAX_KEYBOARD_KEYS] = { 0 };     // Registers previous frame key state

		int keyPressedQueue[MAX_KEY_PRESSED_QUEUE] = { 0 };   // Input keys queue
		int keyPressedQueueCount = 0;                         // Input keys queue count

		int charPressedQueue[MAX_CHAR_PRESSED_QUEUE] = { 0 }; // Input characters queue (unicode)
		int charPressedQueueCount = 0;                        // Input characters queue count
	} Keyboard;

	struct
	{
		glm::vec2 offset = glm::vec2(0.0f);                   // Mouse offset
		glm::vec2 scale = { 1.0f, 1.0f };                     // Mouse scaling
		glm::vec2 currentPosition = glm::vec2(0.0f);          // Mouse position on screen
		glm::vec2 previousPosition = glm::vec2(0.0f);         // Previous mouse position

		int cursor = 0;                                       // Tracks current mouse cursor
		bool cursorHidden = false;                            // Track if cursor is hidden
		bool cursorOnScreen = false;                          // Tracks if cursor is inside client area

		char currentButtonState[MAX_MOUSE_BUTTONS] = { 0 };   // Registers current mouse button state
		char previousButtonState[MAX_MOUSE_BUTTONS] = { 0 };  // Registers previous mouse button state
		glm::vec2 currentWheelMove = glm::vec2(0.0f);         // Registers current mouse wheel variation
		glm::vec2 previousWheelMove = glm::vec2(0.0f);        // Registers previous mouse wheel variation
	} Mouse;

	void UpdateInput()
	{
		Keyboard.keyPressedQueueCount = 0;
		Keyboard.charPressedQueueCount = 0;

		// Register previous keys states
		for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) Keyboard.previousKeyState[i] = Keyboard.currentKeyState[i];

		// Register previous mouse states
		for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) Mouse.previousButtonState[i] = Mouse.currentButtonState[i];

		// Register previous mouse wheel state
		Mouse.previousWheelMove = Mouse.currentWheelMove;
		Mouse.currentWheelMove = { 0.0f, 0.0f };

		// Register previous mouse position
		Mouse.previousPosition = Mouse.currentPosition;

		glfwPollEvents();
	}

	bool IsKeyPressed(int key)
	{
		bool pressed = false;
		if ((Keyboard.previousKeyState[key] == 0) && (Keyboard.currentKeyState[key] == 1)) pressed = true;
		return pressed;
	}
	bool IsKeyDown(int key)
	{
		if (Keyboard.currentKeyState[key] == 1) return true;
		else return false;
	}
	bool IsKeyReleased(int key)
	{
		bool released = false;
		if ((Keyboard.previousKeyState[key] == 1) && (Keyboard.currentKeyState[key] == 0)) released = true;
		return released;
	}
	bool IsKeyUp(int key)
	{
		if (Keyboard.currentKeyState[key] == 0) return true;
		else return false;
	}
	int GetKeyPressed()
	{
		int value = 0;

		if (Keyboard.keyPressedQueueCount > 0)
		{
			// Get character from the queue head
			value = Keyboard.keyPressedQueue[0];

			// Shift elements 1 step toward the head.
			for (int i = 0; i < (Keyboard.keyPressedQueueCount - 1); i++)
				Keyboard.keyPressedQueue[i] = Keyboard.keyPressedQueue[i + 1];

			// Reset last character in the queue
			Keyboard.keyPressedQueue[Keyboard.keyPressedQueueCount] = 0;
			Keyboard.keyPressedQueueCount--;
		}

		return value;
	}
	int GetCharPressed()
	{
		int value = 0;

		if (Keyboard.charPressedQueueCount > 0)
		{
			// Get character from the queue head
			value = Keyboard.charPressedQueue[0];

			// Shift elements 1 step toward the head.
			for (int i = 0; i < (Keyboard.charPressedQueueCount - 1); i++)
				Keyboard.charPressedQueue[i] = Keyboard.charPressedQueue[i + 1];

			// Reset last character in the queue
			Keyboard.charPressedQueue[Keyboard.charPressedQueueCount] = 0;
			Keyboard.charPressedQueueCount--;
		}

		return value;
	}

	bool IsMouseButtonPressed(int button)
	{
		bool pressed = false;
		if ((Mouse.currentButtonState[button] == 1) && (Mouse.previousButtonState[button] == 0)) pressed = true;
		return pressed;
	}
	bool IsMouseButtonDown(int button)
	{
		bool down = false;
		if (Mouse.currentButtonState[button] == 1) down = true;
		return down;
	}
	bool IsMouseButtonReleased(int button)
	{
		bool released = false;
		if ((Mouse.currentButtonState[button] == 0) && (Mouse.previousButtonState[button] == 1)) released = true;
		return released;
	}
	bool IsMouseButtonUp(int button)
	{
		return !IsMouseButtonDown(button);
	}
	int GetMouseX()
	{
		return (int)((Mouse.currentPosition.x + Mouse.offset.x) * Mouse.scale.x);
	}
	int GetMouseY()
	{
		return (int)((Mouse.currentPosition.y + Mouse.offset.y) * Mouse.scale.y);
	}
	glm::vec2 GetMousePosition()
	{
		glm::vec2 position = glm::vec2{ 0 };
		position.x = (Mouse.currentPosition.x + Mouse.offset.x) * Mouse.scale.x;
		position.y = (Mouse.currentPosition.y + Mouse.offset.y) * Mouse.scale.y;
		return position;
	}
	glm::vec2 GetMouseDelta()
	{
		glm::vec2 delta = glm::vec2{ 0 };
		delta.x = Mouse.currentPosition.x - Mouse.previousPosition.x;
		delta.y = Mouse.currentPosition.y - Mouse.previousPosition.y;
		return delta;
	}
	void SetMousePosition(int x, int y)
	{
		Mouse.currentPosition = { (float)x, (float)y };
		glfwSetCursorPos(window, Mouse.currentPosition.x, Mouse.currentPosition.y);
	}
	void SetMouseOffset(int offsetX, int offsetY)
	{
		Mouse.offset = { (float)offsetX, (float)offsetY };
	}
	void SetMouseScale(float scaleX, float scaleY)
	{
		Mouse.scale = { scaleX, scaleY };
	}
	float GetMouseWheelMove()
	{
		float result = 0.0f;
		if (fabsf(Mouse.currentWheelMove.x) > fabsf(Mouse.currentWheelMove.y)) result = (float)Mouse.currentWheelMove.x;
		else result = (float)Mouse.currentWheelMove.y;
		return result;
	}
	glm::vec2 GetMouseWheelMoveV()
	{
		glm::vec2 result = glm::vec2{ 0 };
		result = Mouse.currentWheelMove;
		return result;
	}
	void SetMouseCursor(int cursor)
	{
		Mouse.cursor = cursor;
		//if (cursor == MOUSE_CURSOR_DEFAULT) glfwSetCursor(privateImpl::window, NULL);
		//else
		//{
		//	// NOTE: We are relating internal GLFW enum values to our MouseCursor enum values
		//	glfwSetCursor(privateImpl::window, glfwCreateStandardCursor(0x00036000 + cursor));
		//}
	}

	void EnableCursor()
	{
#if defined(__EMSCRIPTEN__)
		emscripten_exit_pointerlock();
#else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
		Mouse.cursorHidden = false;
	}
	
	void DisableCursor()
	{
#if defined(__EMSCRIPTEN__)
		emscripten_request_pointerlock("#canvas", 1);
#else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
		Mouse.cursorHidden = true;
	}

	bool IsCursorOnScreen()
	{
		return Mouse.cursorOnScreen;
	}

	//-------------------------------------------------------------------------
	// Window
	//-------------------------------------------------------------------------
	GLFWwindow* window = nullptr;
	int FrameBufferWidth = 0;
	int FrameBufferHeight = 0;
	float FrameBufferAspectRatio = 0.0f;
	bool IsEngineExit = true;
		
	void errorCallback(int /*error*/, const char* description) noexcept
	{
		core::LogError("Error: " + std::string(description));
		IsEngineExit = true;
	}

	void windowSizeCallback(GLFWwindow* /*window*/, int width, int height) noexcept
	{
		FrameBufferWidth = base::Max(width, 1);
		FrameBufferHeight = base::Max(height, 1);

		if (FrameBufferHeight < 1) FrameBufferHeight = 1;
		FrameBufferAspectRatio = static_cast<float>(FrameBufferWidth) / static_cast<float>(FrameBufferHeight);
	}

	void cursorEnterCallback(GLFWwindow* /*window*/, int enter) noexcept
	{
		Mouse.cursorOnScreen = (enter == GLFW_TRUE);
	}

	void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods) noexcept
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		if (key < 0) return;    // Security check, macOS fn key generates -1

		// WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1 to work properly with our implementation (IsKeyDown/IsKeyUp checks)
		if (action == GLFW_RELEASE) Keyboard.currentKeyState[key] = 0;
		else Keyboard.currentKeyState[key] = 1;

		// WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
		if (((key == KEY_CAPS_LOCK) && ((mods & GLFW_MOD_CAPS_LOCK) > 0)) ||
			((key == KEY_NUM_LOCK) && ((mods & GLFW_MOD_NUM_LOCK) > 0))) Keyboard.currentKeyState[key] = 1;

		// Check if there is space available in the key queue
		if ((Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS))
		{
			// Add character to the queue
			Keyboard.keyPressedQueue[Keyboard.keyPressedQueueCount] = key;
			Keyboard.keyPressedQueueCount++;
		}
	}

	// GLFW3 Char Key Callback, runs on key down (gets equivalent unicode char value)
	void charCallback(GLFWwindow* /*window*/, unsigned int key) noexcept
	{
		// NOTE: Registers any key down considering OS keyboard layout but
		// do not detects action events, those should be managed by user...
		// Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
		// Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

		// Check if there is space available in the queue
		if (Keyboard.charPressedQueueCount < MAX_KEY_PRESSED_QUEUE)
		{
			// Add character to the queue
			Keyboard.charPressedQueue[Keyboard.charPressedQueueCount] = key;
			Keyboard.charPressedQueueCount++;
		}
	}

	void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) noexcept
	{
		// WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
		// but future releases may add more actions (i.e. GLFW_REPEAT)
		Mouse.currentButtonState[button] = action;
	}

	void mouseCursorPosCallback(GLFWwindow* /*window*/, double x, double y) noexcept
	{
		Mouse.currentPosition.x = (float)x;
		Mouse.currentPosition.y = (float)y;
	}

	void mouseScrollCallback(GLFWwindow* /*window*/, double xoffset, double yoffset) noexcept
	{
		Mouse.currentWheelMove = { (float)xoffset, (float)yoffset };
	}

	bool CreateWindowSystem(const WindowCreateInfo& createInfo)
	{
		assert(!window);

		glfwSetErrorCallback(errorCallback);

		if (!glfwInit())
		{
			core::LogError("GLFW: Failed to initialize GLFW");
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(_DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		glfwWindowHint(GLFW_RESIZABLE, createInfo.Resizable ? GL_TRUE : GL_FALSE);

		window = glfwCreateWindow(createInfo.Width, createInfo.Height, createInfo.Title, NULL, NULL);
		if (!window)
		{
			core::LogError("GLFW: Failed to initialize Window");
			return false;
		}

		glfwSetWindowSizeCallback(window, windowSizeCallback);
		glfwSetKeyCallback(window, keyCallback);
		glfwSetCharCallback(window, charCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, mouseCursorPosCallback);
		glfwSetScrollCallback(window, mouseScrollCallback);
		glfwSetCursorEnterCallback(window, cursorEnterCallback);

		glfwMakeContextCurrent(window);

		if (!gladLoadGL(glfwGetProcAddress))
		{
			core::LogError("GLAD: Cannot load OpenGL extensions");
			return false;
		}

		glfwSwapInterval(createInfo.Vsync ? 1 : 0);

		glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);
		if (FrameBufferWidth == 0 || FrameBufferHeight == 0)
			return false;
		FrameBufferAspectRatio = static_cast<float>(FrameBufferWidth) / static_cast<float>(FrameBufferHeight);

		core::LogPrint("OpenGL device information:");
		core::LogPrint("    > Vendor:   " + std::string((const char*)glGetString(GL_VENDOR)));
		core::LogPrint("    > Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
		core::LogPrint("    > Version:  " + std::string((const char*)glGetString(GL_VERSION)));
		core::LogPrint("    > GLSL:     " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));


		core::LogPrint("OpenGL limits:");
		GLint capability = 0;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &capability);
		core::LogPrint("    > GL_MAX_VERTEX_UNIFORM_COMPONENTS: " + std::to_string(capability));
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &capability);
		core::LogPrint("    > GL_MAX_FRAGMENT_UNIFORM_COMPONENTS : " + std::to_string(capability));
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &capability);
		core::LogPrint("    > GL_MAX_UNIFORM_LOCATIONS: " + std::to_string(capability));

		IsEngineExit = false;

		return true;
	}

	void DestroyWindowSystem()
	{
		IsEngineExit = true;
		glfwDestroyWindow(window);
		window = nullptr;
		glfwTerminate();
	}

	bool WindowShouldClose()
	{
		return IsEngineExit || glfwWindowShouldClose(window) == GLFW_TRUE;
	}

	void UpdateWindow()
	{
		glfwSwapBuffers(window);
	}

	int GetFrameBufferWidth()
	{
		return FrameBufferWidth;
	}
	int GetFrameBufferHeight()
	{
		return FrameBufferHeight;
	}
	float GetFrameBufferAspectRatio()
	{
		return FrameBufferAspectRatio;
	}
}
#pragma endregion

#pragma region Renderer
namespace renderer
{
	namespace
	{
		int RenderWidth = 0;
		int RenderHeight = 0;
		glm::vec3 ClearColor;

		unsigned currentShaderProgram = 0;
		unsigned currentTexture2D[8] = { 0 };
		unsigned currentVAO = 0;
		FrameBuffer* currentFrameBuffer = nullptr;

		glm::mat4 projectionMatrix;
		float perspectiveFOV = 0.0f;
		float perspectiveNear = 0.01f;
		float perspectiveFar = 1000.0f;
	}

#if defined(_DEBUG)
	void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept
	{
		// Ignore non-significant error/warning codes (NVidia drivers)
		// NOTE: Here there are the details with a sample output:
		// - #131169 - Framebuffer detailed info: The driver allocated storage for renderbuffer 2. (severity: low)
		// - #131185 - Buffer detailed info: Buffer object 1 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_ENUM_88e4)
		//             will use VIDEO memory as the source for buffer object operations. (severity: low)
		// - #131218 - Program/shader state performance warning: Vertex shader in program 7 is being recompiled based on GL state. (severity: medium)
		// - #131204 - Texture state usage warning: The texture object (0) bound to texture image unit 0 does not have
		//             a defined base level and cannot be used for texture mapping. (severity: low)
		if ((id == 131169) || (id == 131185) || (id == 131218) || (id == 131204)) return;

		const char* msgSource = nullptr;
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: msgSource = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: msgSource = "WINDOW_SYSTEM"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: msgSource = "SHADER_COMPILER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: msgSource = "THIRD_PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: msgSource = "APPLICATION"; break;
		case GL_DEBUG_SOURCE_OTHER: msgSource = "OTHER"; break;
		default: break;
		}

		const char* msgType = nullptr;
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: msgType = "ERROR"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msgType = "DEPRECATED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: msgType = "UNDEFINED_BEHAVIOR"; break;
		case GL_DEBUG_TYPE_PORTABILITY: msgType = "PORTABILITY"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: msgType = "PERFORMANCE"; break;
		case GL_DEBUG_TYPE_MARKER: msgType = "MARKER"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: msgType = "PUSH_GROUP"; break;
		case GL_DEBUG_TYPE_POP_GROUP: msgType = "POP_GROUP"; break;
		case GL_DEBUG_TYPE_OTHER: msgType = "OTHER"; break;
		default: break;
		}

		const char* msgSeverity = "DEFAULT";
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_LOW: msgSeverity = "LOW"; break;
		case GL_DEBUG_SEVERITY_MEDIUM: msgSeverity = "MEDIUM"; break;
		case GL_DEBUG_SEVERITY_HIGH: msgSeverity = "HIGH"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: msgSeverity = "NOTIFICATION"; break;
		default: break;
		}

		core::LogError("GL: OpenGL debug message: " + std::string(message));
		core::LogError("    > Type: " + std::string(msgType));
		core::LogError("    > Source = " + std::string(msgSource));
		core::LogError("    > Severity = " + std::string(msgSeverity));
	}
#endif

	bool CreateRenderSystem(const RendererCreateInfo& createInfo)
	{
#if defined(_DEBUG)
		if ((glDebugMessageCallback != NULL) && (glDebugMessageControl != NULL))
		{
			glDebugMessageCallback(debugMessageCallback, 0);
			// glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, 0, GL_TRUE); // TODO: Filter message

			// Debug context options:
			//  - GL_DEBUG_OUTPUT - Faster version but not useful for breakpoints
			//  - GL_DEBUG_OUTPUT_SYNCHRONUS - Callback is in sync with errors, so a breakpoint can be placed on the callback in order to get a stacktrace for the GL error
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		}
#endif

		ClearColor = createInfo.ClearColor;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
		glClearDepth(1.0f);
		glViewport(0, 0, platform::GetFrameBufferWidth(), platform::GetFrameBufferHeight());
				
		perspectiveFOV = createInfo.PerspectiveFOV;
		perspectiveNear = createInfo.PerspectiveNear;
		perspectiveFar = createInfo.PerspectiveFar;

		const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / platform::GetFrameBufferAspectRatio()) * 2.0f;
		projectionMatrix = glm::perspective(FOVY, platform::GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);

		return true;
	}

	void DestroyRenderSystem()
	{

	}

	void BeginRenderFrame()
	{
		if (RenderWidth != platform::FrameBufferWidth || RenderHeight != platform::FrameBufferHeight)
		{
			RenderWidth = platform::FrameBufferWidth;
			RenderHeight = platform::FrameBufferHeight;
			glViewport(0, 0, RenderWidth, RenderHeight);
			const float FOVY2 = glm::radians(perspectiveFOV);
			const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / platform::GetFrameBufferAspectRatio()) * 2.0f;
			projectionMatrix = glm::perspective(FOVY, platform::GetFrameBufferAspectRatio(), perspectiveNear, perspectiveFar);

		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	inline GLenum translate(RenderResourceUsage usage)
	{
		switch (usage)
		{
		case RenderResourceUsage::Static:  return GL_STATIC_DRAW;
		case RenderResourceUsage::Dynamic: return GL_DYNAMIC_DRAW;
		case RenderResourceUsage::Stream:  return GL_STREAM_DRAW;
		}
		return 0;
	}

	bool ShaderProgram::CreateFromMemories(const char* vertexShaderMemory, const char* fragmentShaderMemory)
	{
		if (vertexShaderMemory == nullptr || fragmentShaderMemory == nullptr) return false;
		if (vertexShaderMemory == "" || fragmentShaderMemory == "") return false;
		if (m_id > 0) Destroy();

		const GLuint shaderVertex = createShader(GL_VERTEX_SHADER, vertexShaderMemory);
		const GLuint shaderFragment = createShader(GL_FRAGMENT_SHADER, fragmentShaderMemory);

		if (shaderVertex > 0 && shaderFragment > 0)
		{
			m_id = glCreateProgram();
			glAttachShader(m_id, shaderVertex);
			glAttachShader(m_id, shaderFragment);

			glLinkProgram(m_id);
			GLint success = 0;
			glGetProgramiv(m_id, GL_LINK_STATUS, &success);
			if (success == GL_FALSE)
			{
				GLint errorMsgLen;
				glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &errorMsgLen);

				std::vector<GLchar> errorInfo(errorMsgLen);
				glGetProgramInfoLog(m_id, errorInfo.size(), nullptr, &errorInfo[0]);
				core::LogError("OPENGL: Shader program linking failed: " + std::string(&errorInfo[0]));
				glDeleteProgram(m_id);
				m_id = 0;
			}
		}

		glDeleteShader(shaderVertex);
		glDeleteShader(shaderFragment);

		return IsValid();
	}

	void ShaderProgram::Destroy()
	{
		if (m_id > 0)
		{
			if (currentShaderProgram == m_id) UnBind();
			glDeleteProgram(m_id);
			m_id = 0;
		}
	}

	void ShaderProgram::Bind()
	{
		if (currentShaderProgram != m_id)
		{
			currentShaderProgram = m_id;
			glUseProgram(currentShaderProgram);
		}
	}

	void ShaderProgram::UnBind()
	{
		currentShaderProgram = 0;
		glUseProgram(0);
	}

	UniformVariable ShaderProgram::GetUniformVariable(const char* name)
	{
		return { glGetUniformLocation(m_id, name) };
	}

	void ShaderProgram::SetUniform(UniformVariable var, int value)
	{
		glUniform1i(var.id, value);
	}

	void ShaderProgram::SetUniform(UniformVariable var, float value)
	{
		glUniform1f(var.id, value);
	}

	void ShaderProgram::SetUniform(UniformVariable var, const glm::vec2& v)
	{
		glUniform2fv(var.id, 1, glm::value_ptr(v));
	}

	void ShaderProgram::SetUniform(UniformVariable var, float x, float y, float z)
	{
		glUniform3f(var.id, x, y, z);
	}

	void ShaderProgram::SetUniform(UniformVariable var, const glm::vec3& v)
	{
		glUniform3fv(var.id, 1, glm::value_ptr(v));
	}

	void ShaderProgram::SetUniform(UniformVariable var, const glm::vec4& v)
	{
		glUniform4fv(var.id, 1, glm::value_ptr(v));
	}

	void ShaderProgram::SetUniform(UniformVariable var, const glm::mat3& mat)
	{
		glUniformMatrix3fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void ShaderProgram::SetUniform(UniformVariable var, const glm::mat4& mat)
	{
		glUniformMatrix4fv(var.id, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void ShaderProgram::SetUniform(const char* name, int value)
	{
		SetUniform(GetUniformVariable(name), value);
	}
	void ShaderProgram::SetUniform(const char* name, float value)
	{
		SetUniform(GetUniformVariable(name), value);
	}
	void ShaderProgram::SetUniform(const char* name, float x, float y, float z)
	{
		SetUniform(GetUniformVariable(name), x, y, z);
	}
	void ShaderProgram::SetUniform(const char* name, const glm::vec2& v)
	{
		SetUniform(GetUniformVariable(name), v);
	}
	void ShaderProgram::SetUniform(const char* name, const glm::vec3& v)
	{
		SetUniform(GetUniformVariable(name), v);
	}
	void ShaderProgram::SetUniform(const char* name, const glm::vec4& v)
	{
		SetUniform(GetUniformVariable(name), v);
	}
	void ShaderProgram::SetUniform(const char* name, const glm::mat3& mat)
	{
		SetUniform(GetUniformVariable(name), mat);
	}
	void ShaderProgram::SetUniform(const char* name, const glm::mat4& mat)
	{
		SetUniform(GetUniformVariable(name), mat);
	}

	unsigned ShaderProgram::createShader(unsigned type, const char* shaderString) const
	{
		if (shaderString == nullptr || shaderString == "") return 0;

		const GLuint shaderId = glCreateShader(type);
		glShaderSource(shaderId, 1, &shaderString, nullptr);
		glCompileShader(shaderId);

		GLint success = 0;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			GLint infoLogSize;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogSize);
			std::vector<GLchar> errorInfo(infoLogSize);
			glGetShaderInfoLog(shaderId, errorInfo.size(), nullptr, &errorInfo[0]);
			glDeleteShader(shaderId);
			const std::string msg = "Shader compilation failed : " + std::string(&errorInfo[0]) + ", Source: " + shaderString;
			core::LogError(msg);
			return 0;
		}

		return shaderId;
	}

	inline GLint translate(TextureWrapping wrap)
	{
		switch (wrap)
		{
		case TextureWrapping::Repeat:         return GL_REPEAT;
		case TextureWrapping::MirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrapping::Clamp:          return GL_CLAMP_TO_EDGE;
		}
		return 0;
	}

	inline GLint translate(TextureMinFilter filter)
	{
		switch (filter)
		{
		case TextureMinFilter::Nearest:              return GL_NEAREST;
		case TextureMinFilter::Linear:               return GL_LINEAR;
		case TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureMinFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
		case TextureMinFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
		case TextureMinFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
		}
		return 0;
	}

	inline GLint translate(TextureMagFilter filter)
	{
		switch (filter)
		{
		case TextureMagFilter::Nearest: return GL_NEAREST;
		case TextureMagFilter::Linear:  return GL_LINEAR;
		}
		return 0;
	}

	inline bool getTextureFormatType(TexelsFormat inFormat, GLenum textureType, GLenum& format, GLint& internalFormat, GLenum& oglType)
	{
		if (inFormat == TexelsFormat::R_U8)
		{
			format = GL_RED;
			internalFormat = GL_R8;
			oglType = GL_UNSIGNED_BYTE;
		}
		else if (inFormat == TexelsFormat::RG_U8)
		{
			format = GL_RG;
			internalFormat = GL_RG8;
			oglType = GL_UNSIGNED_BYTE;
			const GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
			glTexParameteriv(textureType, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask); // TODO: могут быть проблемы с браузерами, тогда только грузить stb с указанием нужного формата
		}
		else if (inFormat == TexelsFormat::RGB_U8)
		{
			format = GL_RGB;
			internalFormat = GL_RGB8;
			oglType = GL_UNSIGNED_BYTE;
		}
		else if (inFormat == TexelsFormat::RGBA_U8)
		{
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
			oglType = GL_UNSIGNED_BYTE;
		}
		else if (inFormat == TexelsFormat::Depth_U16)
		{
			format = GL_DEPTH_COMPONENT;
			internalFormat = GL_DEPTH_COMPONENT16;
			oglType = GL_UNSIGNED_SHORT;
		}
		else if (inFormat == TexelsFormat::DepthStencil_U16)
		{
			format = GL_DEPTH_STENCIL;
			internalFormat = GL_DEPTH24_STENCIL8;
			oglType = GL_UNSIGNED_SHORT;
		}
		else if (inFormat == TexelsFormat::Depth_U24)
		{
			format = GL_DEPTH_COMPONENT;
			internalFormat = GL_DEPTH_COMPONENT24;
			oglType = GL_UNSIGNED_INT;
		}
		else if (inFormat == TexelsFormat::DepthStencil_U24)
		{
			format = GL_DEPTH_STENCIL;
			internalFormat = GL_DEPTH24_STENCIL8;
			oglType = GL_UNSIGNED_INT;
		}
		else
		{
			core::LogError("unknown texture format");
			return false;
		}
		return true;
	}

	bool Texture2D::CreateFromMemories(const Texture2DCreateInfo& createInfo)
	{
		if (id > 0) Destroy();

		isTransparent = createInfo.isTransparent;

		// save prev pixel store state
		GLint Alignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &Alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// gen texture res
		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, translate(createInfo.wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, translate(createInfo.wrapT));

		// set texture filtering parameters
		TextureMinFilter minFilter = createInfo.minFilter;
		if (!createInfo.mipmap)
		{
			if (createInfo.minFilter == TextureMinFilter::NearestMipmapNearest) minFilter = TextureMinFilter::Nearest;
			else if (createInfo.minFilter != TextureMinFilter::Nearest) minFilter = TextureMinFilter::Linear;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translate(minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translate(createInfo.magFilter));

		// set texture format
		GLenum format = GL_RGB;
		GLint internalFormat = GL_RGB;
		GLenum oglType = GL_UNSIGNED_BYTE;
		getTextureFormatType(createInfo.format, GL_TEXTURE_2D, format, internalFormat, oglType);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, createInfo.width, createInfo.height, 0, format, oglType, createInfo.data);
		if (createInfo.mipmap)
			glGenerateMipmap(GL_TEXTURE_2D);

		// restore prev state
		glBindTexture(GL_TEXTURE_2D, currentTexture2D[0]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);
		return true;
	}

	bool Texture2D::CreateFromFiles(const Texture2DLoaderInfo& loaderInfo)
	{
		if (loaderInfo.fileName == nullptr || loaderInfo.fileName == "") return false;

		Texture2DCreateInfo createInfo(loaderInfo);
		{
			if (loaderInfo.verticallyFlip)
				stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

			int width = 0;
			int height = 0;
			int nrChannels = 0;
			TexelsFormat format = TexelsFormat::RGB_U8;
			uint8_t* data = stbi_load(loaderInfo.fileName, &width, &height, &nrChannels, 0);
			if (!data || nrChannels < STBI_grey || nrChannels > STBI_rgb_alpha || width == 0 || height == 0)
			{
				core::LogError("Texture loading failed! Filename='" + std::string(loaderInfo.fileName) + "'");
				return false;
			}
			if (nrChannels == STBI_grey) format = TexelsFormat::R_U8;
			else if (nrChannels == STBI_grey_alpha) format = TexelsFormat::RG_U8;
			else if (nrChannels == STBI_rgb) format = TexelsFormat::RGB_U8;
			else if (nrChannels == STBI_rgb_alpha) format = TexelsFormat::RGBA_U8;

			// проверить на прозрачность
			// TODO: может быть медленно, проверить скорость и поискать другое решение
			createInfo.isTransparent = false;
			if (format == TexelsFormat::RGBA_U8)
			{
				for (int i = 0; i < width * height * nrChannels; i += 4)
				{
					//uint8_t r = data[i];
					//uint8_t g = data[i + 1];
					//uint8_t b = data[i + 2];
					const uint8_t& a = data[i + 3];
					if (a < 255)
					{
						createInfo.isTransparent = true;
						break;
					}
				}
			}

			createInfo.format = format;
			createInfo.width = static_cast<uint16_t>(width);
			createInfo.height = static_cast<uint16_t>(height);
			createInfo.depth = 1;
			createInfo.data = data;

			bool isValid = CreateFromMemories(createInfo);
			stbi_image_free((void*)data);
			if (!isValid) return false;
		}

		return true;
	}

	void Texture2D::Destroy()
	{
		if (id > 0)
		{
			for (unsigned i = 0; i < 8; i++)
			{
				if (currentTexture2D[i] == id)
					Texture2D::UnBind(i);
			}
			glDeleteTextures(1, &id);
			id = 0;
		}
	}

	void Texture2D::Bind(unsigned slot) const
	{
		if (currentTexture2D[slot] != id)
		{
			currentTexture2D[slot] = id;
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, id);
		}
	}

	void Texture2D::UnBind(unsigned slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, 0);
		currentTexture2D[slot] = 0;
	}

	bool VertexBuffer::Create(RenderResourceUsage usage, unsigned vertexCount, unsigned vertexSize, const void* data)
	{
		if (m_id > 0) Destroy();

		m_vertexCount = vertexCount;
		m_vertexSize = vertexSize;
		m_usage = usage;
		glGenBuffers(1, &m_id);

		GLint currentVBO = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);

		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * m_vertexSize, data, translate(m_usage));

		glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(currentVBO));
		return true;
	}

	void VertexBuffer::Destroy()
	{
		GLint currentVBO = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);
		if (static_cast<GLuint>(currentVBO) == m_id) glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (m_id) glDeleteBuffers(1, &m_id);
		m_id = 0;
	}

	void VertexBuffer::Update(unsigned offset, unsigned size, const void* data)
	{
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
	}

	bool IndexBuffer::Create(RenderResourceUsage usage, unsigned indexCount, unsigned indexSize, const void* data)
	{
		if (m_id > 0) Destroy();

		m_indexCount = indexCount;
		m_indexSize = indexSize;
		m_usage = usage;
		glGenBuffers(1, &m_id);

		GLint currentIBO = 0;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, data, translate(m_usage));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(currentIBO));
		return true;
	}

	void IndexBuffer::Destroy()
	{
		glDeleteBuffers(1, &m_id);
		m_id = 0;
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	}

	inline GLenum translate(PrimitiveDraw p)
	{
		switch (p)
		{
		case PrimitiveDraw::Lines:     return GL_LINES;
		case PrimitiveDraw::Triangles: return GL_TRIANGLES;
		case PrimitiveDraw::Points:    return GL_POINTS;
		}
		return 0;
	}

	inline GLenum translate(VertexAttributeType p)
	{
		switch (p)
		{
		case VertexAttributeType::Float:     return GL_FLOAT;
		}
		return 0;
	}

	bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs)
	{
		if (m_id > 0) Destroy();
		if (!vbo || attribs.empty()) return false;

		m_ibo = ibo;
		m_vbo = vbo;

		glGenVertexArrays(1, &m_id);
		glBindVertexArray(m_id);

		vbo->Bind();
		for (size_t i = 0; i < attribs.size(); i++)
		{
			const auto& att = attribs[i];
			glEnableVertexAttribArray(static_cast<GLuint>(i));
			glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
		}
		m_attribsCount = attribs.size();

		if (m_ibo) m_ibo->Bind();

		glBindVertexArray(currentVAO);
		return true;
	}

	bool VertexArrayBuffer::Create(VertexBuffer* vbo, IndexBuffer* ibo, VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs, const std::vector<VertexAttribute>& instanceAttribs)
	{
		if (!Create(vbo, ibo, attribs))
			return false;

		SetInstancedBuffer(instanceBuffer, instanceAttribs);
		return true;
	}

	bool VertexArrayBuffer::Create(const std::vector<VertexBuffer*> vbo, IndexBuffer* ibo, const std::vector<VertexAttribute>& attribs)
	{
		if (m_id > 0) Destroy();
		if (vbo.size() != attribs.size()) return false;

		m_ibo = ibo;
		m_vbo = vbo[0];

		glGenVertexArrays(1, &m_id);
		glBindVertexArray(m_id);

		for (size_t i = 0; i < vbo.size(); i++)
		{
			const auto& att = attribs[i];
			glEnableVertexAttribArray(i);
			vbo[i]->Bind();
			glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, att.stride, att.pointer);
		}
		if (m_ibo) m_ibo->Bind();

		glBindVertexArray(currentVAO);
		return true;
	}

	void VertexArrayBuffer::Destroy()
	{
		if (m_id > 0 && currentVAO == m_id)
		{
			if (m_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			currentVAO = 0;
			glBindVertexArray(0);
			glDeleteVertexArrays(1, &m_id);
		}
		m_id = 0;
		m_instanceBuffer = nullptr;
	}

	void VertexArrayBuffer::SetInstancedBuffer(VertexBuffer* instanceBuffer, const std::vector<VertexAttribute>& attribs)
	{
		if (m_instanceBuffer == instanceBuffer) return;

		m_instanceBuffer = instanceBuffer;

		while (m_instancedAttribsCount > m_attribsCount)
		{
			glDisableVertexAttribArray(m_instancedAttribsCount);
			m_instancedAttribsCount--;
		}
		m_instancedAttribsCount = m_attribsCount;

#if 0
		// TODO: сейчас это под матрицы, надо как-то сделать чтобы под другие типы тоже
		{
			glBindVertexArray(m_id);
			instanceBuffer->Bind();
			// set attribute pointers for matrix (4 times vec4)
			glEnableVertexAttribArray(m_instancedAttribsCount);
			glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glVertexAttribDivisor(m_instancedAttribsCount, 1);
			m_instancedAttribsCount++;
			glEnableVertexAttribArray(m_instancedAttribsCount);
			glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glVertexAttribDivisor(m_instancedAttribsCount, 1);
			m_instancedAttribsCount++;
			glEnableVertexAttribArray(m_instancedAttribsCount);
			glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glVertexAttribDivisor(m_instancedAttribsCount, 1);
			m_instancedAttribsCount++;
			glEnableVertexAttribArray(m_instancedAttribsCount);
			glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
			glVertexAttribDivisor(m_instancedAttribsCount, 1);
		}
#else
		{
			glBindVertexArray(m_id);
			instanceBuffer->Bind();

			for (size_t i = 0; i < attribs.size(); i++)
			{
				m_instancedAttribsCount = m_instancedAttribsCount + i;
				const auto& att = attribs[i];

				if (att.type == VertexAttributeType::Matrix4)
				{
					glEnableVertexAttribArray(m_instancedAttribsCount);
					glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)0);
					glVertexAttribDivisor(m_instancedAttribsCount, 1);
					m_instancedAttribsCount++;
					glEnableVertexAttribArray(m_instancedAttribsCount);
					glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
					glVertexAttribDivisor(m_instancedAttribsCount, 1);
					m_instancedAttribsCount++;
					glEnableVertexAttribArray(m_instancedAttribsCount);
					glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
					glVertexAttribDivisor(m_instancedAttribsCount, 1);
					m_instancedAttribsCount++;
					glEnableVertexAttribArray(m_instancedAttribsCount);
					glVertexAttribPointer(m_instancedAttribsCount, 4, GL_FLOAT, att.normalized ? GL_TRUE : GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
					glVertexAttribDivisor(m_instancedAttribsCount, 1);
				}
				else
				{
					// TODO: проверить
					glEnableVertexAttribArray(m_instancedAttribsCount);
					glVertexAttribPointer(i, att.size, translate(att.type), att.normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(att.stride), att.pointer);
					glVertexAttribDivisor(m_instancedAttribsCount, 1);
				}
			}
		}
#endif

		glBindVertexArray(currentVAO);
	}

	void VertexArrayBuffer::Draw(PrimitiveDraw primitive, uint32_t instanceCount)
	{
		if (instanceCount == 0) return;

		if (currentVAO != m_id)
		{
			currentVAO = m_id;
			glBindVertexArray(m_id);
		}

		if (m_instanceBuffer)
		{
			if (instanceCount == 1 || instanceCount > m_instanceBuffer->GetVertexCount())
				instanceCount = m_instanceBuffer->GetVertexCount();
		}

		if (m_ibo)
		{
			const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

			if (instanceCount > 1)
				glDrawElementsInstanced(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr, instanceCount);
			else
				glDrawElements(translate(primitive), m_ibo->GetIndexCount(), indexSizeType, nullptr);
				
		}
		else
		{
			if (instanceCount > 1)
				glDrawArraysInstanced(translate(primitive), 0, m_vbo->GetVertexCount(), instanceCount);
			else
				glDrawArrays(translate(primitive), 0, m_vbo->GetVertexCount());
		}
	}

	void VertexArrayBuffer::DrawElementsBaseVertex(PrimitiveDraw primitive, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex)
	{
		if (!m_ibo) return;

		if (currentVAO != m_id)
		{
			currentVAO = m_id;
			glBindVertexArray(m_id);
		}

		const unsigned indexSizeType = m_ibo->GetIndexSize() == sizeof(uint32_t) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		glDrawElementsBaseVertex(translate(primitive), indexCount, indexSizeType, (void*)(m_ibo->GetIndexSize() * baseIndex), baseVertex);
	}

	void VertexArrayBuffer::UnBind()
	{
		currentVAO = 0;
		glBindVertexArray(0);
	}

	bool FrameBuffer::Create(int width, int height)
	{
		if (width < 1 || height < 1) return false;
		m_width = width;
		m_height = height;
		glGenFramebuffers(1, &m_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_id);

		glGenTextures(1, &m_texColorBuffer);
		glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
#if 1
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
#else
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, nullptr);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // TODO: GL_LINEAR 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		Texture2D::UnBind(); // TODO:

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texColorBuffer, 0);

		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			core::LogError("Framebuffer is not complete!");
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		currentFrameBuffer = nullptr;
		const float aspect = (float)width / (float)height;
		const float FOVY = glm::atan(glm::tan(glm::radians(perspectiveFOV) / 2.0f) / aspect) * 2.0f;
		m_projectionMatrix = glm::perspective(FOVY, aspect, perspectiveNear, perspectiveFar);

		return true;
	}

	void FrameBuffer::Destroy()
	{
		if (currentFrameBuffer == this) MainFrameBufferBind();

		glDeleteTextures(1, &m_texColorBuffer);
		glDeleteRenderbuffers(1, &m_rbo);
		glDeleteFramebuffers(1, &m_id);
	}

	void FrameBuffer::Bind(const glm::vec3& color)
	{
		if (currentFrameBuffer != this)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_id);
			glViewport(0, 0, m_width, m_height);
			currentFrameBuffer = this;
		}
		glClearColor(color.x, color.y, color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void FrameBuffer::MainFrameBufferBind()
	{
		if (!currentFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, platform::GetFrameBufferWidth(), platform::GetFrameBufferHeight());
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		currentFrameBuffer = nullptr;
	}

	void FrameBuffer::BindTextureBuffer()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texColorBuffer);
		Texture2D::UnBind(); // TODO:
	}

	const glm::mat4& GetCurrentProjectionMatrix()
	{
		if (currentFrameBuffer) 
			return currentFrameBuffer->GetProjectionMatrix();
		else
			return renderer::projectionMatrix;
	}

	namespace TextureFileManager
	{
		std::unordered_map<std::string, Texture2D> FileTextures;

		void Destroy()
		{
			for (auto it = FileTextures.begin(); it != FileTextures.end(); ++it)
				it->second.Destroy();
			FileTextures.clear();
		}

		Texture2D* LoadTexture2D(const char* name)
		{
			Texture2DLoaderInfo textureLoaderInfo = {};
			textureLoaderInfo.fileName = name;
			return LoadTexture2D(textureLoaderInfo);
		}

		Texture2D* LoadTexture2D(const Texture2DLoaderInfo& textureLoaderInfo)
		{
			auto it = FileTextures.find(textureLoaderInfo.fileName);
			if (it != FileTextures.end())
			{
				return &it->second;
			}
			else
			{
				core::LogPrint("Load texture: " + std::string(textureLoaderInfo.fileName));

				Texture2D texture;
				if (!texture.CreateFromFiles(textureLoaderInfo) || !texture.IsValid())
					return nullptr;

				FileTextures[textureLoaderInfo.fileName] = texture;
				return &FileTextures[textureLoaderInfo.fileName];
			}
		}
	}
}
#pragma endregion

#pragma region Graphics3D
namespace g3d
{

	void Camera::MoveForward(float deltaTime, float speedMod)
	{
		m_position += m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::MoveBackward(float deltaTime, float speedMod)
	{
		m_position -= m_front * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::MoveRight(float deltaTime, float speedMod)
	{
		m_position += m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::MoveLeft(float deltaTime, float speedMod)
	{
		m_position -= m_right * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::MoveUp(float deltaTime, float speedMod)
	{
		m_position += m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::MoveDown(float deltaTime, float speedMod)
	{
		m_position -= m_up * (m_movementSpeed * speedMod * deltaTime);
	}

	void Camera::Rotate(float offsetX, float offsetY)
	{
		m_yaw += offsetX * m_sensitivity;
		m_pitch += offsetY * m_sensitivity;

		if (m_pitch > 89.0f) m_pitch = 89.0f;
		else if (m_pitch < -89.0f) m_pitch = -89.0f;

		if (m_yaw > 360.0f) m_yaw = 0.0f;
		else if (m_yaw < -360.0f) m_yaw = 0.0f;

		updateVectors();
	}

	void Camera::SimpleMove(float deltaTime)
	{
		const float xpos = platform::GetMouseX();
		const float ypos = platform::GetMouseY();
		static float lastPosX = xpos;
		static float lastPosY = ypos;
		Rotate((xpos - lastPosX), (lastPosY - ypos));
		lastPosX = xpos;
		lastPosY = ypos;

		constexpr float speedMod = 1.0f;
		if (platform::IsKeyDown(platform::KEY_W))
			MoveForward(deltaTime, speedMod);
		if (platform::IsKeyDown(platform::KEY_S))
			MoveBackward(deltaTime, speedMod);
		if (platform::IsKeyDown(platform::KEY_D))
			MoveRight(deltaTime, speedMod);
		if (platform::IsKeyDown(platform::KEY_A))
			MoveLeft(deltaTime, speedMod);

		constexpr float speedRotateMod = 1600.0f;
		if (platform::IsKeyDown(platform::KEY_E))
			Rotate(speedRotateMod * deltaTime, 0.0f);
		if (platform::IsKeyDown(platform::KEY_Q))
			Rotate(-speedRotateMod * deltaTime, 0.0f);

#ifdef _DEBUG
		if (IsKeyDown(platform::KEY_T))
			MoveUp(deltaTime, speedMod / 2.0f);
		if (IsKeyDown(platform::KEY_G))
			MoveDown(deltaTime, speedMod / 2.0f);

		if (IsKeyDown(platform::KEY_R))
			Rotate(0.0f, speedRotateMod * deltaTime);
		if (IsKeyDown(platform::KEY_F))
			Rotate(0.0f, -speedRotateMod * deltaTime);
#endif

		Update();
	}

	void Camera::Update()
	{
		m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
	}

	void Camera::SetRotate(float yaw, float pitch)
	{
		m_yaw = yaw;
		m_pitch = pitch;
		if (m_pitch > 89.0f) m_pitch = 89.0f;
		else if (m_pitch < -89.0f) m_pitch = -89.0f;
		if (m_yaw > 360.0f) m_yaw = 0.0f;
		else if (m_yaw < -360.0f) m_yaw = 0.0f;
		updateVectors();
	}

	//Frustum Camera::ComputeFrustum() const
	//{
	//	Frustum frustum;

	//	const float halfVSide = m_far * tanf(m_fov * 0.5f);
	//	const float halfHSide = halfVSide * GetWindowAspect();
	//	const glm::vec3 frontMultFar = m_far * m_front;

	//	frustum.nearFace = { m_position + m_near * m_front, m_front };
	//	frustum.farFace = { m_position + frontMultFar, -m_front };
	//	frustum.rightFace = { m_position, glm::cross(m_up, frontMultFar + m_right * halfHSide) };
	//	frustum.leftFace = { m_position, glm::cross(frontMultFar - m_right * halfHSide, m_up) };
	//	frustum.topFace = { m_position, glm::cross(m_right, frontMultFar - m_up * halfVSide) };
	//	frustum.bottomFace = { m_position, glm::cross(frontMultFar + m_up * halfVSide, m_right) };

	//	return frustum;
	//}

	void Camera::updateVectors()
	{
		const glm::vec3 front = {
			cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
			sin(glm::radians(m_pitch)),
			sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
		};
		m_front = glm::normalize(front);
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

	bool Model::Create(const char* fileName, const char* pathMaterialFiles)
	{
		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = pathMaterialFiles; // Path to material files

		tinyobj::ObjReader reader;
		if (!reader.ParseFromFile(fileName, readerConfig))
		{
			if (!reader.Error().empty()) 
				core::LogError("TinyObjReader: " + reader.Error());
			return false;
		}
		if (!reader.Warning().empty())
			core::LogWarning("TinyObjReader: " + reader.Warning());

		auto& attributes = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		const bool isFindMaterials = !materials.empty();

		std::vector<Mesh> tempMesh(materials.size());
		std::vector<std::unordered_map<renderer::Vertex_Pos3_TexCoord, uint32_t>> uniqueVertices(materials.size());
		if (tempMesh.empty())
		{
			tempMesh.resize(1);
			uniqueVertices.resize(1);
		}

		// Loop over shapes
		for (size_t shapeId = 0; shapeId < shapes.size(); shapeId++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t faceId = 0; faceId < shapes[shapeId].mesh.num_face_vertices.size(); faceId++)
			{
				size_t fv = size_t(shapes[shapeId].mesh.num_face_vertices[faceId]);

				// per-face material
				int materialId = shapes[shapeId].mesh.material_ids[faceId];
				if (materialId < 0) materialId = 0;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shapes[shapeId].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attributes.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attributes.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attributes.vertices[3 * size_t(idx.vertex_index) + 2];

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attributes.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attributes.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attributes.normals[3 * size_t(idx.normal_index) + 2];
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					tinyobj::real_t tx = 0;
					tinyobj::real_t ty = 0;
					if (idx.texcoord_index >= 0)
					{
						tx = attributes.texcoords[2 * size_t(idx.texcoord_index) + 0];
						ty = attributes.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}

					// Optional: vertex colors
					// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

					glm::vec3 position{ vx, vy, vz };
					glm::vec2 texCoord{ tx,ty };
					renderer::Vertex_Pos3_TexCoord vertex{ position, texCoord };

					if (uniqueVertices[materialId].count(vertex) == 0)
					{
						uniqueVertices[materialId][vertex] = static_cast<uint32_t>(tempMesh[materialId].vertices.size());
						tempMesh[materialId].vertices.emplace_back(vertex);
					}

					tempMesh[materialId].indices.emplace_back(uniqueVertices[materialId][vertex]);
				}
				index_offset += fv;
			}
		}

		// load materials
		bool isFindToTransparent = false;
		if (isFindMaterials)
		{
			for (int i = 0; i < materials.size(); i++)
			{
				if (materials[i].diffuse_texname.empty()) continue;

				std::string diffuseMap = pathMaterialFiles + materials[i].diffuse_texname;
				tempMesh[i].material.diffuseTexture = renderer::TextureFileManager::LoadTexture2D(diffuseMap.c_str());
				if (!isFindToTransparent && tempMesh[i].material.diffuseTexture)
					isFindToTransparent = tempMesh[i].material.diffuseTexture->isTransparent;
			}
		}

		// сортировка по прозрачности
		if (isFindToTransparent)
		{
			std::vector<Mesh> tempMesh2;

			// TODO: медленно, оптимизировать

			// сначала непрозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (!tempMesh[i].material.diffuseTexture)
					tempMesh2.push_back(tempMesh[i]);
				else if (!tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}
			// теперь прозрачное
			for (int i = 0; i < tempMesh.size(); i++)
			{
				if (tempMesh[i].material.diffuseTexture->isTransparent)
					tempMesh2.push_back(tempMesh[i]);
			}

			return Create(std::move(tempMesh2));
		}
		else
			return Create(std::move(tempMesh));
	}

	bool g3d::Model::Create(std::vector<Mesh>&& meshes)
	{
		Destroy();
		m_subMeshes = std::move(meshes);
		return createBuffer();
	}

	void Model::Destroy()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			m_subMeshes[i].vertices.clear();
			m_subMeshes[i].indices.clear();

			m_subMeshes[i].vertexBuffer.Destroy();
			m_subMeshes[i].indexBuffer.Destroy();
			m_subMeshes[i].vao.Destroy();
		}
		m_subMeshes.clear();
	}

	void Model::SetInstancedBuffer(renderer::VertexBuffer* instanceBuffer, const std::vector<renderer::VertexAttribute>& attribs)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
				m_subMeshes[i].vao.SetInstancedBuffer(instanceBuffer, attribs);
		}
	}

	void Model::Draw(uint32_t instanceCount)
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (m_subMeshes[i].vao.IsValid())
			{
				const renderer::Texture2D* diffuseTexture = m_subMeshes[i].material.diffuseTexture;
				if (diffuseTexture && diffuseTexture->IsValid())
					diffuseTexture->Bind();
				m_subMeshes[i].vao.Draw(renderer::PrimitiveDraw::Triangles, instanceCount);
			}
		}
	}

	bool Model::createBuffer()
	{
		for (int i = 0; i < m_subMeshes.size(); i++)
		{
			if (!m_subMeshes[i].vertexBuffer.Create(renderer::RenderResourceUsage::Static, m_subMeshes[i].vertices.size(), sizeof(m_subMeshes[i].vertices[0]), m_subMeshes[i].vertices.data()))
			{
				core::LogError("VertexBuffer create failed!");
				Destroy();
				return false;
			}
			if (!m_subMeshes[i].indexBuffer.Create(renderer::RenderResourceUsage::Static, m_subMeshes[i].indices.size(), sizeof(uint32_t), m_subMeshes[i].indices.data()))
			{
				core::LogError("IndexBuffer create failed!");
				Destroy();
				return false;
			}

			if (!m_subMeshes[i].vao.Create(&m_subMeshes[i].vertexBuffer, &m_subMeshes[i].indexBuffer, renderer::GetVertexAttributes<renderer::Vertex_Pos3_TexCoord>()))
			{
				core::LogError("VAO create failed!");
				Destroy();
				return false;
			}
		}
		return true;
	}

	namespace ModelFileManager
	{
		std::unordered_map<std::string, Model> FileModels;

		void Destroy()
		{
			for (auto it = FileModels.begin(); it != FileModels.end(); ++it)
				it->second.Destroy();
			FileModels.clear();
		}

		Model* LoadModel(const char* name)
		{
			auto it = FileModels.find(name);
			if (it != FileModels.end())
			{
				return &it->second;
			}
			else
			{
				core::LogPrint("Load model: " + std::string(name));

				Model model;
				if (!model.Create(name) || !model.IsValid())
					return nullptr;

				FileModels[name] = model;
				return &FileModels[name];
			}
		}
	}

	void drawPrimitive::DrawLine(const Camera& camera, const glm::vec3& startPos, const glm::vec3& endPos)
	{
		static bool isCreate = false;
		static renderer::VertexArrayBuffer vao;
		static renderer::VertexBuffer vertexBuf;
		static renderer::ShaderProgram shaderProgram;
		static renderer::UniformVariable MatrixID;

		if (!isCreate)
		{
			isCreate = true;

			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};

			vertexBuf.Create(renderer::RenderResourceUsage::Dynamic, 2, 3 * sizeof(float), vertexData);

			const std::vector<renderer::VertexAttribute> attribs =
			{
				{.size = 3, .type = renderer::VertexAttributeType::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, nullptr, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
out vec4 outColor;
void main()
{
outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
		}
		else
		{
			const float vertexData[] =
			{
				startPos.x, startPos.y, startPos.z,// 0
				endPos.x, endPos.y,  endPos.z// 1
			};
			vertexBuf.Update(0, sizeof(vertexData), vertexData);
		}

		//const glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix()/* * worldMModelMatrixatrix*/;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		glLineWidth(4);
		glDisable(GL_DEPTH_TEST);
		vao.Draw(renderer::PrimitiveDraw::Lines);
		glLineWidth(1);
		glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const Camera& camera, const glm::mat4& worldMatrix, const glm::vec4& color, bool disableDepthTest)
	{
		static bool isCreate = false;
		static renderer::VertexArrayBuffer vao;
		static renderer::VertexBuffer vertexBuf;
		static renderer::IndexBuffer indexBuf;
		static renderer::ShaderProgram shaderProgram;
		static renderer::UniformVariable MatrixID;
		static renderer::UniformVariable ColorID;

		if (!isCreate)
		{
			isCreate = true;

			constexpr float vertexData[] =
			{
				-0.5f, -0.5f,  0.5f,// 0
					0.5f, -0.5f,  0.5f,// 1
					0.5f,  0.5f,  0.5f,// 2
				-0.5f,  0.5f,  0.5f,// 3
				-0.5f, -0.5f, -0.5f,// 4
					0.5f, -0.5f, -0.5f,// 5
					0.5f,  0.5f, -0.5f,// 6
				-0.5f,  0.5f, -0.5f,// 7
			};
			constexpr uint16_t indexData[] =
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,
				4, 5,
				5, 6,
				6, 7,
				7, 4,
				3, 7,
				2, 6,
				0, 4,
				1, 5
			};

			vertexBuf.Create(renderer::RenderResourceUsage::Static, 8, 3 * sizeof(float), vertexData);
			indexBuf.Create(renderer::RenderResourceUsage::Static, 24, sizeof(uint16_t), indexData);

			const std::vector<renderer::VertexAttribute> attribs =
			{
				{.size = 3, .type = renderer::VertexAttributeType::Float, .normalized = false, .stride = 0, .pointer = (void*)0},
			};
			vao.Create(&vertexBuf, &indexBuf, attribs);

			const char* vertexSource = R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
uniform mat4 MVP;
void main()
{
gl_Position =  MVP * vec4(vertexPosition, 1);
}
)";

			const char* fragmentSource = R"(
#version 330 core
uniform vec4 inColor;
out vec4 outColor;
void main()
{
outColor = inColor;
}
)";
			shaderProgram.CreateFromMemories(vertexSource, fragmentSource);
			shaderProgram.Bind();
			MatrixID = shaderProgram.GetUniformVariable("MVP");
			ColorID = shaderProgram.GetUniformVariable("inColor");
		}

		const glm::mat4 MVP = renderer::GetCurrentProjectionMatrix() * camera.GetViewMatrix() * worldMatrix;

		shaderProgram.Bind();
		shaderProgram.SetUniform(MatrixID, MVP);
		shaderProgram.SetUniform(ColorID, color);
		if (disableDepthTest) glDisable(GL_DEPTH_TEST);
		//glLineWidth(4);
		vao.Draw(renderer::PrimitiveDraw::Lines);
		if (disableDepthTest) glEnable(GL_DEPTH_TEST);
	}

	void drawPrimitive::DrawCubeWires(const Camera& camera, const glm::vec3& position, const glm::vec3& size, const glm::vec3& rotationRadian, const glm::vec4& color, bool disableDepthTest)
	{
		const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), rotationRadian.x, glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), rotationRadian.y, glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), rotationRadian.z, glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 roationMatrix = transformY * transformX * transformZ;

		// translation * rotation * scale (also know as TRS matrix)
		const glm::mat4 ModelMatrix =
			glm::translate(glm::mat4(1.0f), position)
			* roationMatrix
			* glm::scale(glm::mat4(1.0f), size);

		drawPrimitive::DrawCubeWires(camera, ModelMatrix, color, disableDepthTest);
	}

}
#pragma endregion

#pragma region Graphics2D
namespace g2d
{

	class Font
	{
	public:
		uint32_t size = 60;
		std::string fontFileName = "../fonts/OpenSans-Regular.ttf";
		const uint32_t atlasWidth = 1024;
		const uint32_t atlasHeight = 1024;
		//const uint32_t oversampleX = 2;
		//const uint32_t oversampleY =2;
		const uint32_t firstCharENG = ' ';
		const uint32_t charCountENG = '~' - ' ' + 1;
		const uint32_t firstCharRUS = 0x400;
		const uint32_t charCountRUS = 0x452 - 0x400;

		std::unique_ptr<stbtt_packedchar[]> charInfo;
		renderer::Texture2D texture;
	};

	static std::vector<Font> m_cacheFont;
	static renderer::ShaderProgram cacheShader;
	static renderer::UniformVariable m_idAttributeTextColor;
	static renderer::UniformVariable m_idAttributeWorldViewProjMatrix;

	constexpr const char* fontVertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

uniform mat4 worldViewProjMatrix;

out vec2 uv0;

void main()
{
    gl_Position = worldViewProjMatrix * vec4(vertex.xy, 0.0, 1.0);
    uv0 = vertex.zw;
}
)";
	constexpr const char* fontFragmentShaderSource = R"(
#version 330 core

in vec2 uv0;

uniform sampler2D mainTex;
uniform vec3 textColor;

out vec4 fragColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(mainTex, uv0).r);
    fragColor = vec4(textColor, 1.0) * sampled;
}
)";

	struct GlyphInfo
	{
		glm::vec4 positions[4];
		float offsetX = 0;
		float offsetY = 0;
	};

	inline GlyphInfo makeGlyphInfo(Font* font, uint32_t character, float offsetX, float offsetY)
	{
		stbtt_aligned_quad quad;

		int char_index = 0;
		if (character < font->firstCharENG + font->charCountENG)
			char_index = character - font->firstCharENG;
		else
			char_index = character - font->firstCharRUS + font->charCountENG;

		stbtt_GetPackedQuad(font->charInfo.get(), font->atlasWidth, font->atlasHeight, char_index, &offsetX, &offsetY, &quad, 1);

		const int sizeY = font->size / 2;

		GlyphInfo info{};
		info.offsetX = offsetX;
		info.offsetY = offsetY;
		info.positions[0] = { quad.x0, quad.y0 + sizeY, quad.s0, quad.t0 };
		info.positions[1] = { quad.x0, quad.y1 + sizeY, quad.s0, quad.t1 };
		info.positions[2] = { quad.x1, quad.y1 + sizeY, quad.s1, quad.t1 };
		info.positions[3] = { quad.x1, quad.y0 + sizeY, quad.s1, quad.t0 };

		return info;
	}

	Font* getFont(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = nullptr;
		for (int i = 0; i < m_cacheFont.size(); i++)
		{
			if (m_cacheFont[i].fontFileName == fontFileName && m_cacheFont[i].size == fontSize)
			{
				font = &m_cacheFont[i];
				break;
			}
		}
		if (!font)
		{
			Font font_;
			font_.size = fontSize;
			font_.fontFileName = fontFileName;

			std::ifstream file(fontFileName, std::ios::binary | std::ios::ate);
			if (!file.is_open())
			{
				core::LogError("Failed to open file " + fontFileName);
				return nullptr;
			}

			const auto size = file.tellg();
			file.seekg(0, std::ios::beg);
			auto bytes = std::vector<uint8_t>(size);
			file.read(reinterpret_cast<char*>(&bytes[0]), size);
			file.close();

			auto atlasData = new uint8_t[font_.atlasWidth * font_.atlasHeight];

			font_.charInfo = std::make_unique<stbtt_packedchar[]>(font_.charCountENG + font_.charCountRUS);

			stbtt_pack_context context;
			//if (!stbtt_PackBegin(&context, atlasData.get(), font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr))
			//	panic("Failed to initialize font");
			stbtt_PackBegin(&context, atlasData, font_.atlasWidth, font_.atlasHeight, 0, 1, nullptr);

			//stbtt_PackSetOversampling(&context, font_.oversampleX, font_.oversampleY);
			//if (!stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get()))
			//    panic("Failed to pack font");

			//stbtt_PackFontRange(&context, fontData.data(), 0, font_.size, font_.firstChar, font_.charCount, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharENG, font_.charCountENG, font_.charInfo.get());
			stbtt_PackFontRange(&context, bytes.data(), 0, font_.size, font_.firstCharRUS, font_.charCountRUS, font_.charInfo.get() + font_.charCountENG);

			stbtt_PackEnd(&context);

			renderer::Texture2DCreateInfo createInfo;
			createInfo.format = renderer::TexelsFormat::R_U8;
			//createInfo.minFilter = TextureMinFilter::Linear;
			//createInfo.magFilter = TextureMagFilter::Linear;
			createInfo.width = font_.atlasWidth;
			createInfo.height = font_.atlasHeight;
			createInfo.depth = 1;
			createInfo.data = atlasData;
			createInfo.mipmap = false;

			//font_.texture.CreateFromMemories(GL_RGB, GL_RED, GL_UNSIGNED_BYTE, font_.atlasWidth, font_.atlasHeight, atlasData.get());
			font_.texture.CreateFromMemories(createInfo);

			delete[] atlasData;

			m_cacheFont.push_back(std::move(font_));
			font = &m_cacheFont[m_cacheFont.size() - 1];
		}

		return font;
	}

	bool Text::Create(const std::string& fontFileName, uint32_t fontSize)
	{
		Font* font = getFont(fontFileName, fontSize);
		if (!font || !create(font))
		{
			core::LogError("Text not create!");
			return false;
		}

		return true;
	}

	void Text::Destroy()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
	}

	void Text::SetText(const std::wstring& text)
	{
		if (m_font && m_text != text)
		{
			m_text = text;
			std::vector<glm::vec4> vertices;
			std::vector<uint16_t> indexes;

			uint16_t lastIndex = 0;
			float offsetX = 0, offsetY = 0;
			for (auto c : text)
			{
				const auto glyphInfo = makeGlyphInfo(m_font, c, offsetX, offsetY);
				offsetX = glyphInfo.offsetX;
				offsetY = glyphInfo.offsetY;

				vertices.emplace_back(glyphInfo.positions[0]);
				vertices.emplace_back(glyphInfo.positions[1]);
				vertices.emplace_back(glyphInfo.positions[2]);
				vertices.emplace_back(glyphInfo.positions[3]);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 1);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex);
				indexes.push_back(lastIndex + 2);
				indexes.push_back(lastIndex + 3);

				lastIndex += 4;
			}
			indexElementCount = indexes.size();

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indexElementCount, indexes.data(), GL_STATIC_DRAW);
		}
	}

	void Text::Draw(const glm::vec3& position, const glm::vec3& color, const glm::mat4& orthoMat)
	{
		if (m_text.empty() || !m_font || !m_font->texture.IsValid())
			return;

		const glm::mat4 pm = orthoMat * glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

		cacheShader.Bind();
		cacheShader.SetUniform(m_idAttributeTextColor, { color.x, color.y, color.z });
		cacheShader.SetUniform(m_idAttributeWorldViewProjMatrix, pm);

		m_font->texture.Bind(0);
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glDrawElements(GL_TRIANGLES, indexElementCount, GL_UNSIGNED_SHORT, nullptr);
		renderer::VertexArrayBuffer::UnBind();
	}

	bool Text::create(Font* font)
	{
		m_font = font;

		if (!cacheShader.IsValid())
		{
			cacheShader.CreateFromMemories(fontVertexShaderSource, fontFragmentShaderSource);
			cacheShader.Bind();
			m_idAttributeTextColor = cacheShader.GetUniformVariable("textColor");
			m_idAttributeWorldViewProjMatrix = cacheShader.GetUniformVariable("worldViewProjMatrix");
			cacheShader.SetUniform("mainTex", 0);
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vertexBuffer);
		glGenBuffers(1, &indexBuffer);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		return true;
	}

}
#pragma endregion

#pragma region Scene
namespace scene
{
	const glm::mat4& Transform::GetWorldMatrix()
	{
		if (m_update)
		{
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_translation);
			glm::mat4 rotationMatrix = glm::toMat4(m_quaternion);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);

			m_worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

			m_update = false;
		}
		return m_worldMatrix;
	}
}
#pragma endregion

#pragma region Physics
namespace physics
{

}
#pragma endregion

#pragma region World
namespace world
{

}
#pragma endregion

#pragma region Engine
namespace engine
{
	namespace
	{
		constexpr static float MicrosecondsToSeconds = 1.0f / 1000000.0f;
		std::chrono::steady_clock::time_point startTime;
		int64_t frameTimeCurrent = 0;
		int64_t frameTimeLast = 0;
		int64_t frameTimeDelta = 0;
		float deltaTime = 0.0f;
	}


	bool CreateEngine(const EngineCreateInfo& createInfo)
	{
		if (!core::CreateLogSystem(createInfo.Log))
			return false;
		
		if (!platform::CreateWindowSystem(createInfo.Window))
			return false;

		if (!renderer::CreateRenderSystem(createInfo.Render))
			return false;

		startTime = std::chrono::high_resolution_clock::now();

		return true;
	}
	void DestroyEngine()
	{
		g3d::ModelFileManager::Destroy();
		renderer::TextureFileManager::Destroy();
		renderer::DestroyRenderSystem();
		platform::DestroyWindowSystem();
		core::DestroyLogSystem();
	}

	bool IsRunningEngine()
	{
		return !platform::WindowShouldClose();
	}
	void BeginFrameEngine()
	{
		// get delta time
		{
			const auto curTime = std::chrono::high_resolution_clock::now();
			frameTimeCurrent = std::chrono::duration_cast<std::chrono::microseconds>(curTime - startTime).count();
			frameTimeDelta = frameTimeCurrent - frameTimeLast;
			frameTimeLast = frameTimeCurrent;
			deltaTime = static_cast<float>(frameTimeDelta) * MicrosecondsToSeconds;
		}

		renderer::BeginRenderFrame();
	}
	void EndFrameEngine()
	{
		platform::UpdateWindow();
		platform::UpdateInput();
	}

	float GetDeltaTime()
	{
		return deltaTime;
	}
}
#pragma endregion