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
std::vector<MoleculePtr> simulatedMolecules;

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
    bool isFixedInPlace;
    std::vector<Atom> atoms;
    std::vector<std::pair<size_t, size_t>> bonds;
};

MoleculePtr loadMolecule(const std::string &filename, bool isFixedInPlace)
{
    chemfiles::Trajectory file(filename);
    chemfiles::Frame frame = file.read();
    const auto &positions = frame.positions();

    auto molecule = std::make_shared<Molecule> ();
    molecule->isFixedInPlace = isFixedInPlace;
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
        "Molesim [Options]\n"
        "-ligand <file>    Ligand file.\n"
        "-receiver <file>  Receiver file.\n"
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
    std::string receiverFileName;
    std::string ligandFileName;
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
            else if(!strcmp(arg, "-receiver"))
            {
                receiverFileName = argv[++i];
            }
            else if(!strcmp(arg, "-ligand"))
            {
                ligandFileName = argv[++i];
            }
            else
            {
                printHelp();
                return 1;
            }
        }

    }

    if (receiverFileName.empty() && ligandFileName.empty())
    {
        printHelp();
        return 0;
    }

    if(!receiverFileName.empty())
    {
        auto receiverMolecule = loadMolecule(receiverFileName, true);
        if(!receiverMolecule)
            return 1;

        printf("Receiver loaded\n");
        simulatedMolecules.push_back(receiverMolecule);
    }

    if(!ligandFileName.empty())
    {
        auto ligandMolecule = loadMolecule(ligandFileName, false);
        if(!ligandMolecule)
            return 1;
        printf("Ligand loaded\n");
        simulatedMolecules.push_back(ligandMolecule);
    }

    return 0;
}