#pragma once

void SetClipboardText(const char* text); // Set clipboard text content
const char* GetClipboardText();          // Get clipboard text content. NOTE: returned string is allocated and freed by GLFW