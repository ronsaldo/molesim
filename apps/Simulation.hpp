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
#include "AGPU/agpu.hpp"

namespace Molesim
{
typedef std::shared_ptr<struct Molecule> MoleculePtr;
typedef std::shared_ptr<struct Simulation> SimulationPtr;

struct AtomRenderingState
{
    Vector3 position;
    float radius;
    Vector4 color;
};

struct AtomDescription
{
    float charge;
    float mass;
    uint32_t atomicNumber;
};

struct ModelState
{
    Matrix4x4 modelMatrix;
    Matrix4x4 inverseModelMatrix;
};

struct Molecule
{
    RigidTransform transform;
    std::vector<AtomDescription> atomDescriptions;
    std::vector<AtomRenderingState> atomStates;
    std::vector<std::pair<size_t, size_t>> bonds;

    agpu_buffer_ref modelStateBuffer;
    agpu_buffer_ref moleculeRenderingStateBuffer;
    agpu_shader_resource_binding_ref moleculeResourceBinding;

    void translateToCenterOfMass();
};

struct Simulation
{
    std::vector<MoleculePtr> molecules;
};

void initializeAtomColorConventions();
MoleculePtr loadMolecule(const std::string &filename);

} // End of namespace Molesim

#endif //MOLEVIS_SIMULATION_HPP