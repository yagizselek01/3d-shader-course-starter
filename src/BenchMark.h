#pragma once

#include <string>
#include <vector>

class Benchmark
{
public:
    void start(const std::string& label);

    void updateWarmup();
    void addSample(double milliseconds);

    bool isRunning() const;
    bool isWarmingUp() const;
    bool isFinished() const;

private:
    static constexpr int WarmupFrameCount = 120;
    static constexpr int TargetSampleCount = 600;

    int warmupFrames = 0;

    bool running = false;
    bool finished = false;

    std::string benchmarkLabel;
    std::vector<double> samples;

    void printResults();
};