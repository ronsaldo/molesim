#include "Simulation.hpp"
#include <stdio.h>
#include <vector>
#include <utility>
#include <time.h>

using namespace Molesim;

// TODO: Use a performance counter in Windows.
double getCurrentTimeMicroseconds()
{
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        return 0.0;

    return ts.tv_sec*1000000.0 + ts.tv_nsec / 1000.0;

}

static int RandomAtomCounts[] = {
    2, 10, 50, 100,
    200, 300, 400, 500, 600, 700, 800, 900,
    1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000
};

static const int SimulationIterationCount = 1000;
static bool PerformPhysicsUpdate = true;
static bool PerformEnergyOptimization = false;


static SpatialSubdivisionAlgorithm SpatialSubdivisionAlgorithms[] = {
    SpatialSubdivisionAlgorithm::Naive,
    SpatialSubdivisionAlgorithm::Grid,
    SpatialSubdivisionAlgorithm::KDTree,
    SpatialSubdivisionAlgorithm::Octree,
    SpatialSubdivisionAlgorithm::BVH
};

std::pair<double, double> benchmarkAlgorithmWithSize(SpatialSubdivisionAlgorithm algorithm, int size)
{
    std::vector<double> iterationTimes;

    auto simulation = std::make_shared<Simulation> ();
    simulation->spatialSubdivisionAlgorithm = algorithm;
    simulation->createMoleculesWithRandomAtoms(size);

    for(size_t i = 0; i < SimulationIterationCount; ++i)
    {
        const auto SimulationTimeStep = 1.0f / 60.0f;
        auto startTime = getCurrentTimeMicroseconds();
        if(PerformPhysicsUpdate)
            simulation->update(SimulationTimeStep);
        
        if(PerformEnergyOptimization)
            simulation->performOptimizationSteps();
        auto endTime = getCurrentTimeMicroseconds();
        auto deltaTime = endTime - startTime;
        iterationTimes.push_back(deltaTime);
    }

    // Average time.
    double timeSum = 0.0;
    for(auto time : iterationTimes)
        timeSum += time;
    double averageTime = timeSum / iterationTimes.size();

    // Std
    double s2 = 0;
    for(auto time : iterationTimes)
    {
        auto delta = time - averageTime;
        s2 += delta*delta;
    }
    s2 /= iterationTimes.size() - 1;
    double std = sqrt(s2);

    return std::make_pair(averageTime, std);
}

void benchmarkAlgorithm(FILE *results, SpatialSubdivisionAlgorithm algorithm)
{
    std::vector<double> averageTimes;
    std::vector<double> stdTimes;
    printf("Benchmark %s\n", spatialSubdivisionAlgorithmToString(algorithm));
    for(int N : RandomAtomCounts)
    {
        auto averageAndStd = benchmarkAlgorithmWithSize(algorithm, N);
        averageTimes.push_back(averageAndStd.first);
        stdTimes.push_back(averageAndStd.second);
    }

    // Average times
    fprintf(results, "%s", spatialSubdivisionAlgorithmToString(algorithm));
    for(auto average : averageTimes)
        fprintf(results, ",%f", average);
    fprintf(results, "\n");

    // Std times.
    fprintf(results, "%s Std", spatialSubdivisionAlgorithmToString(algorithm));
    for(auto std : stdTimes)
        fprintf(results, ",%f", std);
    fprintf(results, "\n");
}

int main()
{
    FILE *results = fopen("benchmark-results.csv", "wb");
    if(!results)
    {
        perror("Failed to open benchmark-results.csv");
        return 1;
    }

    // Header
    fprintf(results, "Algorithm");
    for(auto N : RandomAtomCounts)
        fprintf(results, ",%d", N);
    fprintf(results, "\n");

    // Per algorithm benchmark
    for(auto algorithm: SpatialSubdivisionAlgorithms)
        benchmarkAlgorithm(results, algorithm);

    fclose(results);
    return 0;
}