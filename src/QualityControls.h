#pragma once

struct GLFWwindow;

enum class QualityMode
{
    Simple = 0,
    Full = 1,
    Auto = 2
};

void updateQualityControls(
    GLFWwindow* window,
    QualityMode& qualityMode
);