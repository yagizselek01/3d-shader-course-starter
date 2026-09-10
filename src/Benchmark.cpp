#include "Benchmark.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <iterator>

void Benchmark::start(const std::string& label)
{
    samples.clear();

    warmupFrames = 0;
    running = true;

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

    // Reject invalid timer results rather than polluting
    // the benchmark statistics.
    if (!std::isfinite(milliseconds) || milliseconds < 0.0)
    {
        std::cerr<< "Invalid GPU timing sample ignored.\n";

        return;
    }

    samples.push_back(milliseconds);

    if (samples.size() >= TargetSampleCount)
    {
        running = false;

        printResults();
    }
}

bool Benchmark::isRunning() const
{
    return running;
}

bool Benchmark::isWarmingUp() const
{
    return running && warmupFrames < WarmupFrameCount;
}

void Benchmark::printResults()
{
    if (samples.empty())
    {
        return;
    }

    // ----- Mean -----

    const double total =
        std::accumulate(
            samples.begin(),
            samples.end(),
            0.0
        );

    const double average = total / static_cast<double>(samples.size());

    // ----- Median -----

    std::vector<double> sortedSamples = samples;

    std::sort(sortedSamples.begin(), sortedSamples.end());

    double median = 0.0;

    const std::size_t count = sortedSamples.size();

    if (count % 2 == 0)
    {
        median =(sortedSamples[count / 2 - 1] + sortedSamples[count / 2]) * 0.5;
    }
    else
    {
        median = sortedSamples[count / 2];
    }

    // ----- Standard deviation -----

    double squaredDifferenceSum = 0.0;

    for (const double sample : samples)
    {
        const double difference = sample - average;

        squaredDifferenceSum += difference * difference;
    }

    const double variance = squaredDifferenceSum / static_cast<double>(samples.size());

    const double standardDeviation = std::sqrt(variance);

    // ----- 5% Trimmed Mean -----
    // Remove the lowest 5% and highest 5% of timing samples.
    // This reduces the influence of occasional GPU timing spikes
    // while still keeping 90% of the measured data.

    const std::size_t trimCount = sortedSamples.size() / 20;

    const auto trimmedBegin = sortedSamples.begin() + static_cast<std::ptrdiff_t>(trimCount);

    const auto trimmedEnd = sortedSamples.end() - static_cast<std::ptrdiff_t>(trimCount);

    const double trimmedTotal =std::accumulate(trimmedBegin, trimmedEnd, 0.0);

    const std::size_t trimmedSampleCount = sortedSamples.size() - (trimCount * 2);

    const double trimmedAverage = trimmedTotal / static_cast<double>(trimmedSampleCount);

    // ----- Min / Max -----

    const auto [minimum, maximum] = std::minmax_element(samples.begin(), samples.end());

    std::cout
        << std::fixed
        << std::setprecision(6)
        << "\n--- GPU BENCHMARK RESULT ---\n"
        << "Mode:              "
        << benchmarkLabel << '\n'
        << "Samples:           "
        << samples.size() << '\n'
        << "Average:           "
        << average << " ms\n"
        << "Median:            "
        << median << " ms\n"
        << "Trimmed mean (5%): "
        << trimmedAverage << " ms\n"
        << "Std. deviation:    "
        << standardDeviation << " ms\n"
        << "Minimum:           "
        << *minimum << " ms\n"
        << "Maximum:           "
        << *maximum << " ms\n"
        << "----------------------------\n";
}