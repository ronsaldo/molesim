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
    size_t firstAtomIndex;
    size_t secondAtomIndex; 

    Vector3 firstRelativePoint;
    Vector3 secondRelativePoint;

    void computeNormalAndPenetrationDistance();
    Scalar computeInverseInertia();
    Scalar computeInverseLinearInertia();
    Scalar computeInverseAngularInertia();
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
    Scalar angularMovementLimit = 0.2f;

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
    void translateBy(const Vector3 &translation);
    void translateByAndRotateBy(const Vector3 &translation, const Vector3 &angularIncrement);

    void applyMovementAtRelativePoint(Scalar movement, const Vector3 &relativePoint, const Vector3 &normalDirection);
    void applyImpulse(const Vector3 &impulse);
    void applyImpulseInRelativePosition(const Vector3 &impulse, const Vector3 &relativePoint);

    void translateToCenterOfMass();
    void computeBoundingBox();
    void computeInertiaTensor();
    void updateWorldInertiaTensor();
    void prepareForSimulation();

    void createFirstTestMolecule();
    void createSecondTestMolecule();
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
    void emitContactPoint(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule, size_t firstAtomIndex, size_t secondAtomIndex);

    void resolveContactManifoldsCollisionsAndConstraints();
    void resolveContactCollisionResponse(ContactPoint &contact);
    void resolveContactConstraint(ContactPoint &contact, Scalar relaxationFactor);


    void update(float deltaTime);
};

void initializeAtomColorConventions();
MoleculePtr loadMolecule(const std::string &filename);

} // End of namespace Molesim

#endif //MOLEVIS_SIMULATION_HPP