#pragma once

struct GLFWwindow;

enum class DebugMode
{
    Final = 0,
    RawFresnel = 1,
    FresnelRim = 2,
    HashBreakup = 3,
    ValueNoise = 4,
    UnwarpedScanlines = 5,
    WarpedScanlines = 6,
    FinalWithoutWarp = 7
};

void updateDebugControls(GLFWwindow* window,DebugMode& debugMode);