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
#include "AABox.hpp"
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

struct ContactPoint
{
    Vector3 normal;
    Scalar penetrationDistance;
    Molecule *firstMolecule;
    Molecule *secondMolecule;
};

struct Molecule
{
    RigidTransform transform;

    Scalar linearDamping = 0.2f;
    Scalar angularDamping = 0.2f;

    Vector3 linearVelocity;
    Vector3 linearVelocityIntegrationDelta;
    Vector3 angularVelocity;
    Vector3 angularVelocityIntegrationDelta;

    Vector3 netForce;
    Vector3 netTorque;
    float totalMass = 0.0f;
    float inverseTotalMass = 0.0;

    Matrix3x3 inertiaTensor;
    Matrix3x3 inverseInertiaTensor;
    Matrix3x3 worldInertiaTensor;
    Matrix3x3 worldInverseInertiaTensor;

    AABox boundingBox;

    std::vector<AtomDescription> atomDescriptions;
    std::vector<AtomRenderingState> atomStates;
    std::vector<std::pair<size_t, size_t>> bonds;

    agpu_buffer_ref modelStateBuffer;
    agpu_buffer_ref moleculeRenderingStateBuffer;
    agpu_shader_resource_binding_ref moleculeResourceBinding;

    void resetNetForces();
    void integrateMovement(float deltaTime);
    void setPositionAndOrientation(const Vector3 &newPosition, const Quaternion &newOrientation);

    void translateToCenterOfMass();
    void computeBoundingBox();
    void computeInertiaTensor();
    void updateWorldInertiaTensor();
    void prepareForSimulation();
};

struct Simulation
{
    std::vector<MoleculePtr> molecules;
    std::vector<ContactPoint> contactPoints;

    void resetNetForces();
    void evaluateForceGenerators(float deltaTime);
    void integrateMovement(float deltaTime);
    void detectAndResolveCollisions();
    std::vector<std::pair<MoleculePtr, MoleculePtr>> computeBroadphase();
    void computeNarrowPhase(const std::vector<std::pair<MoleculePtr, MoleculePtr>> &broadphasePairs);
    void computePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    void computeNaivePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    void emitContactPoint(const Vector3 &firstAtomWorldPosition, Scalar firstAtomRadius, Vector3 &secondAtomWorldPosition, Scalar secondAtomRadius, const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);

    void update(float deltaTime);
};

void initializeAtomColorConventions();
MoleculePtr loadMolecule(const std::string &filename);

} // End of namespace Molesim

#endif //MOLEVIS_SIMULATION_HPP