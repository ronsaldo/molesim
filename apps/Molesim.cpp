#include "Simulation.hpp"
#include <stdio.h>
#include <vector>
#include <string>

using namespace Molesim;

void printHelp()
{
    printf("Molesim <options> <molecule-files>");
}

void printVersion()
{
    printf("Molesim version 0.1");
}

int main(int argc, const char **argv)
{
    std::vector<std::string> moleculeFileNames;
    bool useNaiveCollisionDetection = false;
    size_t simulationIterationCount = 1000;

    for(int i = 1; i < argc; ++i)
    {
        auto arg = argv[i];
        if(*arg == '-')
        {
            if(!strcmp(arg, "-help"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(arg, "-version"))
            {
                printVersion();
                return 0;
            }
            else if(!strcmp(arg, "-iterations"))
            {
                simulationIterationCount = atoi(argv[++i]);
            }
            else if(!strcmp(arg, "-naive-collisions"))
            {
                useNaiveCollisionDetection = true;
            }
            else
            {
                printHelp();
                return 1;
            }
        }
        else
        {
            moleculeFileNames.push_back(arg);
        }
    }

    auto simulation = std::make_shared<Simulation> ();
    simulation->useNaiveNarrowphase = useNaiveCollisionDetection;
    initializeAtomColorConventions();
    if(moleculeFileNames.empty())
    {
        {
            auto firstMolecule = std::make_shared<Molecule> ();
            firstMolecule->createFirstTestMolecule();
            simulation->molecules.push_back(firstMolecule);
        }

        {
            auto secondMolecule = std::make_shared<Molecule> ();
            secondMolecule->createSecondTestMolecule();
            simulation->molecules.push_back(secondMolecule);
        }
    }
    else
    {
        for(auto &fileName : moleculeFileNames)
        {
            auto molecule = loadMolecule(fileName);
            if(!molecule)
                return 1;
            molecule->prepareForSimulation();
            simulation->molecules.push_back(molecule);
        }
    }

    for(size_t i = 0; i < simulationIterationCount; ++i)
    {
        const auto SimulationTimeStep = 1.0f / 60.0f;
        simulation->update(SimulationTimeStep);
    }

    return 0;
}