#include "QualityControls.h"

#include <GLFW/glfw3.h>
#include <iostream>

void updateQualityControls(GLFWwindow* window, QualityMode& qualityMode)
{
    static bool f1WasPressed = false;
    static bool f2WasPressed = false;
    static bool f3WasPressed = false;

    const bool f1Pressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;

    const bool f2Pressed = glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS;

    const bool f3Pressed = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;

    if (f1Pressed && !f1WasPressed)
    {
        qualityMode = QualityMode::Simple;
        std::cout << "Quality mode: Simple\n";
    }

    if (f2Pressed && !f2WasPressed)
    {
        qualityMode = QualityMode::Full;
        std::cout << "Quality mode: Full\n";
    }

    if (f3Pressed && !f3WasPressed)
    {
        qualityMode = QualityMode::Auto;
        std::cout << "Quality mode: Auto\n";
    }

    f1WasPressed = f1Pressed;
    f2WasPressed = f2Pressed;
    f3WasPressed = f3Pressed;
}