#include "GpuTimer.h"

GpuTimer::GpuTimer()
{
    glGenQueries(1, &queryId);
}

GpuTimer::~GpuTimer()
{
    if (queryId != 0)
    {
        glDeleteQueries(1, &queryId);
    }
}

bool GpuTimer::begin()
{
    // Do not reuse the query until the previous GPU result
    // has been collected.
    if (measurementPending || queryActive)
    {
        return false;
    }

    glBeginQuery(GL_TIME_ELAPSED, queryId);

    queryActive = true;

    return true;
}

void GpuTimer::end()
{
    if (!queryActive)
    {
        return;
    }

    glEndQuery(GL_TIME_ELAPSED);

    queryActive = false;
    measurementPending = true;
}

bool GpuTimer::tryGetElapsedMilliseconds(double& milliseconds)
{
    if (!measurementPending)
    {
        return false;
    }

    GLint resultAvailable = GL_FALSE;

    glGetQueryObjectiv(
        queryId,
        GL_QUERY_RESULT_AVAILABLE,
        &resultAvailable
    );

    if (resultAvailable == GL_FALSE)
    {
        return false;
    }

    GLuint64 elapsedNanoseconds = 0;

    glGetQueryObjectui64v(
        queryId,
        GL_QUERY_RESULT,
        &elapsedNanoseconds
    );

    milliseconds =
        static_cast<double>(elapsedNanoseconds) /
        1'000'000.0;

    measurementPending = false;

    return true;
}