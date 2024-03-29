#include "stdafx.h"
#include "BaseHeader.h"
#include "Input.h"
#include "Window.h"
//-------------------------------------------------------------------------
extern GLFWwindow* window;
extern KeyCallback KeyEvent;
extern CharCallback CharEvent;
extern MouseButtonCallback MouseButtonEvent;
extern MouseMoveCallback MouseMoveEvent;
extern MouseScrollCallback MouseScrollEvent;
extern MouseCursorEnterCallback MouseCursorEnterEvent;
//-------------------------------------------------------------------------
constexpr auto MAX_KEYBOARD_KEYS = 512;     // Maximum number of keyboard keys supported
constexpr auto MAX_KEY_PRESSED_QUEUE = 16;  // Maximum number of keys in the key input queue
constexpr auto MAX_CHAR_PRESSED_QUEUE = 16; // Maximum number of characters in the char input queue
constexpr auto MAX_MOUSE_BUTTONS = 8;       // Maximum number of mouse buttons supported
//-------------------------------------------------------------------------
struct
{
	char currentKeyState[MAX_KEYBOARD_KEYS] = { 0 };      // Registers current frame key state
	char previousKeyState[MAX_KEYBOARD_KEYS] = { 0 };     // Registers previous frame key state

	int keyPressedQueue[MAX_KEY_PRESSED_QUEUE] = { 0 };   // Input keys queue
	int keyPressedQueueCount = 0;                         // Input keys queue count

	int charPressedQueue[MAX_CHAR_PRESSED_QUEUE] = { 0 }; // Input characters queue (unicode)
	int charPressedQueueCount = 0;                        // Input characters queue count
} Keyboard;
//-------------------------------------------------------------------------
struct
{
	glm::vec2 offset = glm::vec2(0.0f);                   // Mouse offset
	glm::vec2 scale = { 1.0f, 1.0f };                     // Mouse scaling
	glm::vec2 currentPosition = glm::vec2(0.0f);          // Mouse position on screen
	glm::vec2 previousPosition = glm::vec2(0.0f);         // Previous mouse position

	bool cursorHidden = false;                            // Track if cursor is hidden
	bool cursorOnScreen = false;                          // Tracks if cursor is inside client area

	char currentButtonState[MAX_MOUSE_BUTTONS] = { 0 };   // Registers current mouse button state
	char previousButtonState[MAX_MOUSE_BUTTONS] = { 0 };  // Registers previous mouse button state
	glm::vec2 currentWheelMove = glm::vec2(0.0f);         // Registers current mouse wheel variation
	glm::vec2 previousWheelMove = glm::vec2(0.0f);        // Registers previous mouse wheel variation
} Mouse;
//-----------------------------------------------------------------------------
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods) noexcept
{
	if (key < 0) return;    // Security check, macOS fn key generates -1

	// WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1 to work properly with our implementation (IsKeyDown/IsKeyUp checks)
	if (action == GLFW_RELEASE) Keyboard.currentKeyState[key] = 0;
	else Keyboard.currentKeyState[key] = 1;

#if defined(_WIN32) || defined(__linux__)
	// WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
	if (((key == KEY_CAPS_LOCK) && ((mods & GLFW_MOD_CAPS_LOCK) > 0)) ||
		((key == KEY_NUM_LOCK) && ((mods & GLFW_MOD_NUM_LOCK) > 0))) Keyboard.currentKeyState[key] = 1;
#endif

	// Check if there is space available in the key queue
	if ((Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS))
	{
		// Add character to the queue
		Keyboard.keyPressedQueue[Keyboard.keyPressedQueueCount] = key;
		Keyboard.keyPressedQueueCount++;
	}
}
//-----------------------------------------------------------------------------
void keyCallbackUser(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
{
	keyCallback(window, key, scancode, action, mods);
	KeyEvent(key, mods, action != GLFW_RELEASE);
}
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void charCallbackUser(GLFWwindow* window, unsigned int key) noexcept
{
	charCallback(window, key);
	CharEvent(key);
}
//-----------------------------------------------------------------------------
void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) noexcept
{
	// WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
	// but future releases may add more actions (i.e. GLFW_REPEAT)
	Mouse.currentButtonState[button] = action;
}
//-----------------------------------------------------------------------------
void mouseButtonCallbackUser(GLFWwindow* window, int button, int action, int mods) noexcept
{
	mouseButtonCallback(window, button, action, mods);
	MouseButtonEvent(button, action);
}
//-----------------------------------------------------------------------------
void mouseCursorPosCallback(GLFWwindow* /*window*/, double x, double y) noexcept
{
	Mouse.currentPosition.x = (float)x;
	Mouse.currentPosition.y = (float)y;
}
//-----------------------------------------------------------------------------
void mouseCursorPosCallbackUser(GLFWwindow* window, double x, double y) noexcept
{
	mouseCursorPosCallback(window, x, y);
	MouseMoveEvent(static_cast<int>(x), static_cast<int>(y));
}
//-----------------------------------------------------------------------------
void mouseScrollCallback(GLFWwindow* /*window*/, double xoffset, double yoffset) noexcept
{
	Mouse.currentWheelMove = { (float)xoffset, (float)yoffset };
}
//-----------------------------------------------------------------------------
void mouseScrollCallbackUser(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	mouseScrollCallback(window, xoffset, yoffset);
	MouseScrollEvent(xoffset, yoffset);
}
//-----------------------------------------------------------------------------
void cursorEnterCallback(GLFWwindow* /*window*/, int enter) noexcept
{
	Mouse.cursorOnScreen = (enter == GLFW_TRUE);
}
//-------------------------------------------------------------------------
void cursorEnterCallbackUser(GLFWwindow* window, int enter) noexcept
{
	cursorEnterCallback(window, enter);
	MouseCursorEnterEvent(enter == GLFW_TRUE);
}
//-----------------------------------------------------------------------------
void InitInput(GLFWwindow* window)
{
	// Set input callback events
	glfwSetKeyCallback(window, KeyEvent ? keyCallbackUser : keyCallback);
	glfwSetCharCallback(window, CharEvent ? charCallbackUser : charCallback);
	glfwSetMouseButtonCallback(window, MouseButtonEvent ? mouseButtonCallbackUser : mouseButtonCallback);
	glfwSetCursorPosCallback(window, MouseMoveEvent ? mouseCursorPosCallbackUser : mouseCursorPosCallback);	
	glfwSetScrollCallback(window, MouseScrollEvent ? mouseScrollCallbackUser : mouseScrollCallback);
	glfwSetCursorEnterCallback(window, MouseCursorEnterEvent ?cursorEnterCallbackUser : cursorEnterCallback);

	extern int FrameBufferWidth;
	extern int FrameBufferHeight;
	Mouse.currentPosition.x = (float)FrameBufferWidth / 2.0f;
	Mouse.currentPosition.y = (float)FrameBufferHeight / 2.0f;
}
//-----------------------------------------------------------------------------
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
//-------------------------------------------------------------------------
bool IsKeyboardKeyPressed(int key)
{
	if ((Keyboard.previousKeyState[key] == 0) && (Keyboard.currentKeyState[key] == 1)) return true;
	return false;
}
//-------------------------------------------------------------------------
bool IsKeyboardKeyDown(int key)
{
	if (Keyboard.currentKeyState[key] == 1) return true;
	else return false;
}
//-------------------------------------------------------------------------
bool IsKeyboardKeyReleased(int key)
{
	if ((Keyboard.previousKeyState[key] == 1) && (Keyboard.currentKeyState[key] == 0)) return true;
	return false;
}
//-------------------------------------------------------------------------
bool IsKeyboardKeyUp(int key)
{
	if (Keyboard.currentKeyState[key] == 0) return true;
	else return false;
}
//-------------------------------------------------------------------------
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
//-------------------------------------------------------------------------
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
//-------------------------------------------------------------------------
bool IsMouseButtonPressed(int button)
{
	if ((Mouse.currentButtonState[button] == 1) && (Mouse.previousButtonState[button] == 0)) return true;
	return false;
}
//-------------------------------------------------------------------------
bool IsMouseButtonDown(int button)
{
	if (Mouse.currentButtonState[button] == 1) return true;
	return false;
}
//-------------------------------------------------------------------------
bool IsMouseButtonReleased(int button)
{
	if ((Mouse.currentButtonState[button] == 0) && (Mouse.previousButtonState[button] == 1)) return true;
	return false;
}
//-------------------------------------------------------------------------
bool IsMouseButtonUp(int button)
{
	return !IsMouseButtonDown(button);
}
//-------------------------------------------------------------------------
int GetMouseX()
{
	return (int)((Mouse.currentPosition.x + Mouse.offset.x) * Mouse.scale.x);
}
//-------------------------------------------------------------------------
int GetMouseY()
{
	return (int)((Mouse.currentPosition.y + Mouse.offset.y) * Mouse.scale.y);
}
//-------------------------------------------------------------------------
glm::vec2 GetMousePosition()
{
	return
	{
		(Mouse.currentPosition.x + Mouse.offset.x) * Mouse.scale.x,
		(Mouse.currentPosition.y + Mouse.offset.y) * Mouse.scale.y,
	};
}
//-------------------------------------------------------------------------
glm::vec2 GetMouseDelta()
{
	return
	{
		Mouse.currentPosition.x - Mouse.previousPosition.x,
		Mouse.currentPosition.y - Mouse.previousPosition.y
	};
}
//-------------------------------------------------------------------------
void SetMousePosition(int x, int y)
{
	Mouse.currentPosition = { (float)x, (float)y };
	glfwSetCursorPos(window, Mouse.currentPosition.x, Mouse.currentPosition.y);
}
//-------------------------------------------------------------------------
void SetMouseOffset(int offsetX, int offsetY)
{
	Mouse.offset = { (float)offsetX, (float)offsetY };
}
//-------------------------------------------------------------------------
void SetMouseScale(float scaleX, float scaleY)
{
	Mouse.scale = { scaleX, scaleY };
}
//-------------------------------------------------------------------------
float GetMouseWheelMove()
{
	float result = 0.0f;
	if (fabsf(Mouse.currentWheelMove.x) > fabsf(Mouse.currentWheelMove.y)) result = (float)Mouse.currentWheelMove.x;
	else result = (float)Mouse.currentWheelMove.y;
	return result;
}
//-------------------------------------------------------------------------
glm::vec2 GetMouseWheelMoveV()
{
	glm::vec2 result = glm::vec2{ 0 };
	result = Mouse.currentWheelMove;
	return result;
}
//-------------------------------------------------------------------------
void SetMouseLock(bool lock)
{
	Mouse.cursorHidden = lock;
#if defined(__EMSCRIPTEN__)
	if (lock)
		emscripten_request_pointerlock("#canvas", 1);
	else
		emscripten_exit_pointerlock();
#else
	glfwSetInputMode(window, GLFW_CURSOR, lock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
#endif
}
//-------------------------------------------------------------------------
bool IsCursorOnScreen()
{
	return Mouse.cursorOnScreen;
}
//-------------------------------------------------------------------------