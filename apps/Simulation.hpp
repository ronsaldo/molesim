#ifndef MOLEVIS_SIMULATION_HPP
#define MOLEVIS_SIMULATION_HPP

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include "Vector3.hpp"
#include "Quaternion.hpp"
#include "RigidTransform.hpp"
#include "AABox.hpp"
#include "BVH.hpp"
#include "Sphere.hpp"
#include "AGPU/agpu.hpp"

namespace Molesim
{
typedef BoundingVolumeHierachy<size_t> MoleculeBVH;
typedef std::shared_ptr<struct Molecule> MoleculePtr;
typedef std::shared_ptr<struct Simulation> SimulationPtr;

/**
 * Sybyl enum taken from UDock2
 */
enum class Sybyl
{
    CarbonSp3,                         // (C.3)
    CarbonSp2,                         // (C.2)
    CarbonAromatic,                    // (C.ar)
    Carbocation,                       // (guanadinium) (C.cat)
    NitrogenSp3,                       // (N.3)
    NitrogenSp2,                       // (N.2)
    NitrogenSp3PositivelyCharged,      // (N.4)
    NitrogenAromatic,                  // (N.ar)
    NitrogenAmide,                     // (N.am)
    NitrogenTrigonalPlanar,            // (N.pl3)
    OxygenSp3,                         // (O.3)
    OxygenSp2,                         // (O.2)
    OxygenInCarboxylatesAndPhosphates, // (O.co2)
    SulphurSp3,                        // (S.3)
    PhosphorusSp3,                     // (P.3)
    Fluorine,                          // (F)
    Hydrogen,                          // (H)
    Lithium                            // (Li)
};

std::string sybyl_toString(Sybyl type);
Sybyl sybyl_fromString( const std::string &type);
float sybyl_getRadius( Sybyl type );
float sybyl_getEpsilon( Sybyl type );

struct AtomRenderingState
{
    Vector3 position;
    float radius;
    Vector4 color;
};

struct AtomDescription
{
    float charge = 0;
    float mass = 0;
    float epsilon = 0;
    Sybyl sybyl;
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
    
    Matrix3x3 computeContactSpaceMatrix() const;
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
    Scalar restitutionCoefficient = 0.2f;
	Scalar dynamicFrictionCoefficient = 0.5f;
	Scalar staticFrictionCoefficient = 0.6f;

    Matrix3x3 inertiaTensor;
    Matrix3x3 inverseInertiaTensor;
    Matrix3x3 worldInertiaTensor;
    Matrix3x3 worldInverseInertiaTensor;

    AABox boundingBox;

    std::vector<AtomDescription> atomDescriptions;
    std::vector<AtomRenderingState> atomStates;
    std::vector<std::pair<size_t, size_t>> bonds;

    MoleculeBVH bvh;

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
    void computeBVH();
    void prepareForSimulation();

    Scalar computeAngularInertiaForRelativeContactPoint(const Vector3 &relativePoint, const Vector3 &normal) const;
    Matrix3x3 computeVelocityPerImpulseWorldMatrixForRelativeContactPoint(const Vector3 &relativePoint) const;
    Vector3 computeVelocityAtRelativePoint(const Vector3 &relativePoint);

    void createFirstTestMolecule();
    void createSecondTestMolecule();
};

struct Simulation
{
    std::vector<MoleculePtr> molecules;
    std::vector<ContactPoint> contactPoints;
    Scalar totalEnergy = 0.0;

    Scalar restingContactVelocityLimit = 0.1f;
    bool useNaiveNarrowphase = false;

    Scalar energyMaxRadiusDefault      = 12.f;
    Scalar optimizationStepSizeDefault = 1.f;

    void resetNetForces();
    void evaluateForceGenerators(float deltaTime);
    void integrateMovement(float deltaTime);
    void detectAndResolveCollisions();
    std::vector<std::pair<MoleculePtr, MoleculePtr>> computeBroadphase();

    void computeNarrowPhase(const std::vector<std::pair<MoleculePtr, MoleculePtr>> &broadphasePairs);
    void computePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    void computeNaivePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    void computeBVHPairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);

    void emitContactPoint(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule, size_t firstAtomIndex, size_t secondAtomIndex);

    void computeTotalEnergy(const std::vector<std::pair<MoleculePtr, MoleculePtr>> &broadphasePairs);
    Scalar computePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    Scalar computeNaivePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);
    Scalar computeBVHPairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule);

    void resolveContactManifoldsCollisionsAndConstraints();
    void resolveContactCollisionResponse(ContactPoint &contact);
    void resolveContactConstraint(ContactPoint &contact, Scalar relaxationFactor);


    void update(float deltaTime);
};

void initializeAtomColorConventions();
MoleculePtr loadMolecule(const std::string &filename);

} // End of namespace Molesim

#endif //MOLEVIS_SIMULATION_HPP