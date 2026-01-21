#pragma once
#include <GLFW/glfw3.h>

inline void LockMouse(GLFWwindow* windowPtr, int x, int y)
{
        glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(windowPtr, x, y);
}

inline void UnlockMouse(GLFWwindow* windowPtr)
{
    glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

inline bool IsMouseLocked(GLFWwindow* windowPtr)
{
    int result = glfwGetInputMode(windowPtr, GLFW_CURSOR);
    if (result == GLFW_CURSOR_DISABLED)
    {
        return true;
    }
    return false;
}