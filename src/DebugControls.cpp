#include "DebugControls.h"

#include <GLFW/glfw3.h>
#include <iostream>

void updateDebugControls(
    GLFWwindow* window,
    DebugMode& debugMode)
{
    constexpr int DebugModeCount = 8;

    static bool wasPressed[DebugModeCount] = {};

    for (int mode = 0; mode < DebugModeCount; ++mode)
    {
        const int key = GLFW_KEY_0 + mode;

        const bool isPressed =
            glfwGetKey(window, key) == GLFW_PRESS;

        if (isPressed && !wasPressed[mode])
        {
            debugMode =
                static_cast<DebugMode>(mode);

            std::cout
                << "Debug mode: "
                << mode
                << '\n';
        }

        wasPressed[mode] = isPressed;
    }
}