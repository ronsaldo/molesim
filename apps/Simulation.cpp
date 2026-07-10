#include "Simulation.hpp"
#include "Random.hpp"
#include <chemfiles.hpp>
#include <unordered_map>

namespace Molesim
{

static Random randColor;
static std::unordered_map<std::string, Vector4> atomTypeColorMap;

void initializeAtomColorConventions()
{
    atomTypeColorMap["H"] = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
    atomTypeColorMap["C"] = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
    atomTypeColorMap["N"] = Vector4(0.1f, 0.1f, 0.8f, 1.0f);
    atomTypeColorMap["O"] = Vector4(0.8f, 0.1f, 0.1f, 1.0f);
}

Vector4
getOrCreateColorForAtomType(const std::string &type)
{
    auto it = atomTypeColorMap.find(type);
    if(it != atomTypeColorMap.end())
        return it->second;
    
    auto generatedColor = randColor.randVector4(Vector4{0.1f, 0.1f, 0.1f, 1.0f}, Vector4{0.8f, 0.8f, 0.8f, 1.0f});
    atomTypeColorMap.insert(std::make_pair(type, generatedColor));
    return generatedColor;
}

MoleculePtr loadMolecule(const std::string &filename)
{
    chemfiles::Trajectory file(filename);
    chemfiles::Frame frame = file.read();
    const auto &positions = frame.positions();

    auto molecule = std::make_shared<Molecule> ();
    molecule->atomDescriptions.reserve(positions.size());
    molecule->atomStates.reserve(positions.size());

    for(size_t i = 0; i < positions.size(); ++i)
    {
        const auto &atomPosition = positions[i];
        const auto &chemAtom = frame[i];

        auto chemAtomNumber = chemAtom.atomic_number();

        auto atomDesc = AtomDescription();
        atomDesc.charge = float(chemAtom.charge());
        atomDesc.mass = chemAtom.mass();
        atomDesc.atomicNumber = chemAtomNumber ? chemAtomNumber.value() : 1;
        molecule->atomDescriptions.push_back(atomDesc);

        auto atomState = AtomRenderingState();
        atomState.position = Vector3(atomPosition[0], atomPosition[1], atomPosition[2]);
        //printf("Position: %f %f %f\n", atomPosition[0], atomPosition[1], atomPosition[2]);
        
        auto chemRadius = chemAtom.covalent_radius();
        atomState.radius = chemRadius ? chemRadius.value() : 0.2f;
        atomState.color = getOrCreateColorForAtomType(chemAtom.type());
        
        molecule->atomStates.push_back(atomState);
    }

    auto &topology = frame.topology();
    for(auto &bond : topology.bonds())
    {
        size_t firstAtomIndex = bond[0];
        size_t secondAtomIndex = bond[1];
        molecule->bonds.push_back(std::make_pair(firstAtomIndex, secondAtomIndex));
    }

    printf("Molecule atom count: %zu\n", molecule->atomStates.size());
    //printf("Molecule bond count: %zu\n", molecule->bonds.size());

    return molecule;
}

void Molecule::translateToCenterOfMass()
{
    Vector3 centerOfMass = Vector3(0);
    totalMass = 0;
    for(size_t i = 0; i < atomStates.size(); ++i)
    {
        float mass = atomDescriptions[i].mass;
        centerOfMass += atomStates[i].position*mass;
        totalMass += mass;
    }

    centerOfMass /= totalMass;
    printf("Center of mass: %f %f %f\n", centerOfMass.x, centerOfMass.y, centerOfMass.z);
    for(auto &state : atomStates)
        state.position -= centerOfMass;

    inverseTotalMass = 1.0f / totalMass;
}

void Molecule::computeBoundingBox()
{
    boundingBox = AABox::Empty();
    for(auto &atom : atomStates)
    {
        AABox atomBoundingBox = AABox::ForSphere(atom.position, atom.radius);
        boundingBox.insertBox(atomBoundingBox);
    }

    printf("Molecule bbox: %f %f %f - %f %f %f\n",
        boundingBox.minCorner.x, boundingBox.minCorner.y, boundingBox.minCorner.z,
        boundingBox.maxCorner.x, boundingBox.maxCorner.y, boundingBox.maxCorner.z);

}

void Molecule::computeInertiaTensor()
{
    // Use the aabox
    // TODO: Use the steiner theorem for something more accurate.
    auto halfExtent = boundingBox.halfExtent();
    auto extent = halfExtent* Vector3(2.0);
    auto extent2 = extent*extent;
    inertiaTensor = Matrix3x3(
        (extent2.y + extent2.z)/12*totalMass, 0, 0,
        0, (extent2.x + extent2.z)/12*totalMass, 0,
        0, 0, (extent2.x + extent2.y)/12*totalMass
    );
    inverseInertiaTensor = inertiaTensor.inverse();

    updateWorldInertiaTensor();
}

void Molecule::updateWorldInertiaTensor()
{
    auto rotationMatrix = transform.rotation.asMatrix();
    auto transposedRotationMatrix = rotationMatrix.transposed();

	worldInertiaTensor = rotationMatrix * inertiaTensor * transposedRotationMatrix;
	worldInverseInertiaTensor = rotationMatrix * inverseInertiaTensor * transposedRotationMatrix;
}

void Molecule::prepareForSimulation()
{
    translateToCenterOfMass();
    computeBoundingBox();
    computeInertiaTensor();
}

void Molecule::resetNetForces()
{
    netForce = Vector3::Zeros();
    netTorque = Vector3::Zeros();
}

void Molecule::integrateMovement(float deltaTime)
{
    // Compute the linear acceleration
    auto linearAcceleration = netForce*Vector3(inverseTotalMass);

    // Integrate the linear acceleration
    auto integratedVelocity = linearVelocity + linearAcceleration*Vector3(deltaTime);

    // Apply the linear damping
    auto linearDampingFactor = clamp(powf(1.0f - linearDamping, deltaTime), 0.0f, 1.0f);
    integratedVelocity *= linearDampingFactor;

    // Integrate the linear velocity
    linearVelocityIntegrationDelta = integratedVelocity - linearVelocity;
    linearVelocity = integratedVelocity;

    // Compute the angular acceleration.
    auto angularAcceleration = worldInverseInertiaTensor * netTorque;

    // Integrate the angular acceleration.
    auto integratedAngularVelocity = angularVelocity + angularAcceleration*Vector3(deltaTime);

    // Apply the angular damping
    auto angularDampingFactor = clamp(powf(1.0f - angularDamping, deltaTime), 0.0f, 1.0f);
    integratedAngularVelocity *= angularDampingFactor;

    // Integrate the angular velocity
    angularVelocityIntegrationDelta = integratedAngularVelocity - angularVelocity;
    angularVelocity = integratedAngularVelocity;

    // Integrate the position.
    auto integratedPosition = transform.translation + linearVelocity*Vector3(deltaTime);

    // Integrate the orientation
    auto integratedOrientation = Quaternion(angularVelocity*Vector3(0.5f*deltaTime)).exp() * transform.rotation;
    integratedOrientation = integratedOrientation.normalized();

    //printf("Linear velocity %f %f %f\n", linearVelocity.x, linearVelocity.y, linearVelocity.z);
    //printf("Angular velocity %f %f %f\n", angularVelocity.x, angularVelocity.y, angularVelocity.z);

    setPositionAndOrientation(integratedPosition, integratedOrientation);
    //printf("damping %f %f\n", linearDampingFactor, angularDampingFactor);
    //transform.translation += linearVelocity *deltaTime;
}

void Molecule::setPositionAndOrientation(const Vector3 &newPosition, const Quaternion &newOrientation)
{
    transform = RigidTransform::WithRotationAndTranslation(newOrientation, newPosition);
    updateWorldInertiaTensor();
}

void Simulation::resetNetForces()
{
    // Update the molecules themselves.
    for(auto &molecule : molecules)
        molecule->resetNetForces();
}

void Simulation::evaluateForceGenerators(float deltaTime)
{
}

void Simulation::integrateMovement(float deltaTime)
{
    // Update the molecules themselves.
    for(auto &molecule : molecules)
        molecule->integrateMovement(deltaTime);

}

void Simulation::detectAndResolveCollisions()
{
    auto broadphasePairs = computeBroadphase();
    printf("broadphasePairs %zu\n", broadphasePairs.size());
}

std::vector<std::pair<MoleculePtr, MoleculePtr>> Simulation::computeBroadphase()
{
    std::vector<std::pair<MoleculePtr, MoleculePtr>> broadphasePairs;
    for(size_t i = 0; i < molecules.size(); ++i)
    {
        auto &firstMolecule = molecules[i];
        auto firstBoundingBox = firstMolecule->boundingBox.transformedWith(firstMolecule->transform);
        for(size_t j = i + 1; j < molecules.size(); ++j)
        {
            auto &secondMolecule = molecules[j];
            auto secondBoundingBox = secondMolecule->boundingBox.transformedWith(secondMolecule->transform);
            if(firstBoundingBox.hasIntersectionWithBox(secondBoundingBox))
                broadphasePairs.push_back(std::make_pair(firstMolecule, secondMolecule));
        }
    }

    return broadphasePairs;
}

void Simulation::update(float deltaTime)
{
    resetNetForces();
    evaluateForceGenerators(deltaTime);
    integrateMovement(deltaTime);
    detectAndResolveCollisions();
}

} // End of namespace Molesim
