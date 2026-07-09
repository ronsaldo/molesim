#ifndef MOLEVIS_SIMULATION_HPP
#define MOLEVIS_SIMULATION_HPP

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include "Vector3.hpp"
#include "Quaternion.hpp"
#include "RigidTransform.hpp"
#include "Sphere.hpp"

namespace Molesim
{
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

MoleculePtr loadMolecule(const std::string &filename);

} // End of namespace Molesim

#endif //MOLEVIS_SIMULATION_HPP