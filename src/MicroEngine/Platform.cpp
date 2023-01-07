#include "stdafx.h"
#include "Platform.h"
//-------------------------------------------------------------------------
void SetClipboardText(const char* text)
{
#if defined(_WIN32) || defined(__linux__)
	extern GLFWwindow* window;
	glfwSetClipboardString(window, text);
#endif
#if defined(__EMSCRIPTEN__)
	emscripten_run_script(TextFormat("navigator.clipboard.writeText('%s')", text));
#endif
}
//-------------------------------------------------------------------------
const char* GetClipboardText()
{
#if defined(_WIN32) || defined(__linux__)
	extern GLFWwindow* window;
	return glfwGetClipboardString(window);
#endif
#if defined(__EMSCRIPTEN__)
	// Accessing clipboard data from browser is tricky due to security reasons
	// The method to use is navigator.clipboard.readText() but this is an asynchronous method
	// that will return at some moment after the function is called with the required data
	emscripten_run_script_string("navigator.clipboard.readText() \
.then(text => { document.getElementById('clipboard').innerText = text; console.log('Pasted content: ', text); }) \
.catch(err => { console.error('Failed to read clipboard contents: ', err); });"
);

	// The main issue is getting that data, one approach could be using ASYNCIFY and wait
	// for the data but it requires adding Asyncify emscripten library on compilation

	// Another approach could be just copy the data in a HTML text field and try to retrieve it
	// later on if available... and clean it for future accesses

	return NULL;
#endif
	return nullptr;
}
//-------------------------------------------------------------------------