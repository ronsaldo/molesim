#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <chemfiles.hpp>
#include "Vector3.hpp"
#include "Quaternion.hpp"
#include "RigidTransform.hpp"
#include "Sphere.hpp"

using namespace Molesim;

typedef std::shared_ptr<struct Molecule> MoleculePtr;
typedef std::shared_ptr<struct Simulation> SimulationPtr;

struct Atom
{
    Vector3 position;
    float radius;
    float charge;
    uint32_t atomicNumber;
};

struct Molecule
{
    RigidTransform transform;
    std::vector<Atom> atoms;
    std::vector<std::pair<size_t, size_t>> bonds;
};

struct Simulation
{
    std::vector<MoleculePtr> molecules;
};

MoleculePtr loadMolecule(const std::string &filename)
{
    chemfiles::Trajectory file(filename);
    chemfiles::Frame frame = file.read();
    const auto &positions = frame.positions();

    auto molecule = std::make_shared<Molecule> ();
    molecule->atoms.reserve(positions.size());

    for(size_t i = 0; i < positions.size(); ++i)
    {
        const auto &atomPosition = positions[i];
        const auto &chemAtom = frame[i];

        auto chemAtomNumber = chemAtom.atomic_number();

        auto atom = Atom();
        atom.position = Vector3(atomPosition[0], atomPosition[1], atomPosition[2]);
        atom.charge = float(chemAtom.charge());
        molecule->atoms.push_back(atom);
    }

    auto &topology = frame.topology();
    for(auto &bond : topology.bonds())
    {
        size_t firstAtomIndex = bond[0];
        size_t secondAtomIndex = bond[1];
        molecule->bonds.push_back(std::make_pair(firstAtomIndex, secondAtomIndex));
    }

    //printf("Molecule atom count: %zu\n", molecule->atoms.size());
    //printf("Molecule bond count: %zu\n", molecule->bonds.size());

    return molecule;
}

void printHelp()
{
    printf(
        "Molesim [Options] <molecules>\n"
        "-h                Print help.\n"
        "-version          Print version.\n"
    );
}

void printVersion()
{
    printf("Molesim version 0.1\n");
}

int main(int argc, const char **argv)
{
    std::vector<std::string> moleculeFileNames;
    SimulationPtr simulation;
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

    if (moleculeFileNames.empty())
    {
        printHelp();
        return 0;
    }

    simulation = std::make_shared<Simulation> ();
    for(auto &fileName : moleculeFileNames)
    {
        auto molecule = loadMolecule(fileName);
        if(!molecule)
            return 1;
        simulation->molecules.push_back(molecule);
    }

    return 0;
}