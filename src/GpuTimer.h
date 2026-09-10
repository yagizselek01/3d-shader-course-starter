#pragma once

#include <glad/gl.h>

class GpuTimer
{
public:
    GpuTimer();
    ~GpuTimer();

    GpuTimer(const GpuTimer&) = delete;
    GpuTimer& operator=(const GpuTimer&) = delete;

    bool begin();
    void end();

    bool tryGetElapsedMilliseconds(double& milliseconds);

private:
    GLuint queryId = 0;

    bool measurementPending = false;
    bool queryActive = false;
};