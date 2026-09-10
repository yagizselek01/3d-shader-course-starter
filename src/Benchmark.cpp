#include "Benchmark.h"

#include <algorithm>
#include <iostream>
#include <numeric>

void Benchmark::start(const std::string& label)
{
    samples.clear();

    warmupFrames = 0;
    running = true;
    finished = false;

    benchmarkLabel = label;

    std::cout
        << "\nGPU benchmark started: "
        << benchmarkLabel
        << '\n';
}

void Benchmark::updateWarmup()
{
    if (!running || !isWarmingUp())
    {
        return;
    }

    ++warmupFrames;
}

void Benchmark::addSample(double milliseconds)
{
    if (!running || isWarmingUp())
    {
        return;
    }

    samples.push_back(milliseconds);

    if (samples.size() >= TargetSampleCount)
    {
        running = false;
        finished = true;

        printResults();
    }
}

bool Benchmark::isRunning() const
{
    return running;
}

bool Benchmark::isWarmingUp() const
{
    return running &&
        warmupFrames < WarmupFrameCount;
}

bool Benchmark::isFinished() const
{
    return finished;
}

void Benchmark::printResults()
{
    if (samples.empty())
    {
        return;
    }

    const double total =
        std::accumulate(
            samples.begin(),
            samples.end(),
            0.0
        );

    const double average =
        total / static_cast<double>(samples.size());

    const auto [minimum, maximum] =
        std::minmax_element(
            samples.begin(),
            samples.end()
        );

    std::cout
        << "\n--- GPU BENCHMARK RESULT ---\n"
        << "Mode: "
        << benchmarkLabel << '\n'
        << "Samples: "
        << samples.size() << '\n'
        << "Average: "
        << average << " ms\n"
        << "Minimum: "
        << *minimum << " ms\n"
        << "Maximum: "
        << *maximum << " ms\n"
        << "----------------------------\n";
}