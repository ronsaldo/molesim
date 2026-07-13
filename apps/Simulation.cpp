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

Scalar Molecule::computeAngularInertiaForRelativeContactPoint(const Vector3 &relativePoint, const Vector3 &normal) const
{
    auto torquePerUnitImpulse = relativePoint.cross(normal);
	auto rotationPerUnitImpulse = worldInverseInertiaTensor * torquePerUnitImpulse;
	return (rotationPerUnitImpulse.cross(relativePoint)).dot(normal);
}

Matrix3x3 Molecule::computeVelocityPerImpulseWorldMatrixForRelativeContactPoint(const Vector3 &relativePoint) const
{
    auto relativePointCrossMatrix = Matrix3x3::SkewSymmetric(relativePoint);
    auto torquePerUnitImpulse = relativePointCrossMatrix;
    auto rotationPerUnitImpulse = worldInverseInertiaTensor * torquePerUnitImpulse;
    return -(relativePointCrossMatrix * rotationPerUnitImpulse);
}

Vector3 Molecule::computeVelocityAtRelativePoint(const Vector3 &relativePoint)
{
    return linearVelocity + angularVelocity.cross(relativePoint);
}

void Molecule::createFirstTestMolecule()
{
    {
        AtomDescription atomDescription;
        atomDescription.mass = 1.0;
        atomDescriptions.push_back(atomDescription);

        AtomRenderingState atomState;
        atomState.position = Vector3();
        atomState.radius = 1.0;
        atomState.color = Vector4(0.8f, 0.1f, 0.1f, 1.0f);
        atomStates.push_back(atomState);
    }
    prepareForSimulation();
    transform.translation = Vector3(-0.25, 0.0, 0.0);
}

void Molecule::createSecondTestMolecule()
{
    {
        AtomDescription atomDescription;
        atomDescription.mass = 1.0;
        atomDescriptions.push_back(atomDescription);

        AtomRenderingState atomState;
        atomState.position = Vector3();
        atomState.radius = 1.0;
        atomState.color = Vector4(0.8f, 0.1f, 0.1f, 1.0f);
        atomStates.push_back(atomState);
    }
    prepareForSimulation();
    transform.translation = Vector3(0.25, 0.0, 0.0);
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

void Molecule::translateBy(const Vector3 &translation)
{
    transform.translation += translation;
}

void Molecule::translateByAndRotateBy(const Vector3 &translation, const Vector3 &angularIncrement)
{
    transform.translation += translation;
    transform.rotation = (Quaternion(angularIncrement * Vector3(0.5)).exp() * transform.rotation).normalized();
    updateWorldInertiaTensor();
}

void Molecule::applyMovementAtRelativePoint(Scalar movement, const Vector3 &relativePoint, const Vector3 &movementDirection)
{
    auto linearMovement = movement * inverseTotalMass;
    auto angularDirection = worldInverseInertiaTensor * relativePoint.cross(movementDirection);
    auto angularFactor = angularDirection.length();
    if(closeTo(angularFactor, 0))
        return translateBy(movementDirection*Vector3(linearMovement));

    angularDirection = angularDirection / angularFactor;
    auto angularMovement = movement*angularFactor;
    if(abs(angularMovement) <= 0)
        return translateBy(movementDirection*Vector3(linearMovement));

    if(abs(angularMovement) > angularMovementLimit)
    {
        angularMovement = angularMovement >= 0 ? angularMovementLimit : -angularMovementLimit;
        auto angularSpentMovement = angularMovement / angularFactor; 

        linearMovement = (movement - angularSpentMovement)*inverseTotalMass;
    }

    //printf("Linear movement %f\n", linearMovement);
    translateByAndRotateBy(movementDirection*Vector3(linearMovement), angularDirection*Vector3(angularMovement));
}

void Molecule::applyImpulse(const Vector3 &impulse)
{
    linearVelocity += impulse*Vector3(inverseTotalMass);
}

void Molecule::applyImpulseInRelativePosition(const Vector3 &impulse, const Vector3 &relativePoint)
{
    linearVelocity += impulse*Vector3(inverseTotalMass);
	angularVelocity += worldInverseInertiaTensor * (relativePoint.cross(impulse));
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
    computeNarrowPhase(broadphasePairs);
    resolveContactManifoldsCollisionsAndConstraints();
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

void Simulation::computeNarrowPhase(const std::vector<std::pair<MoleculePtr, MoleculePtr>> &broadphasePairs)
{
    contactPoints.clear();
    for(auto &pair: broadphasePairs)
        computePairNarrowPhase(pair.first, pair.second);
    //printf("Contact points: %zu\n", contactPoints.size());
}

void Simulation::computePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    computeNaivePairNarrowPhase(firstMolecule, secondMolecule);
}

void Simulation::computeNaivePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    for(size_t i = 0; i < firstMolecule->atomStates.size(); ++i)
    {
        auto &firstAtom = firstMolecule->atomStates[i];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        for(size_t j = 0; j < secondMolecule->atomStates.size(); ++j)
        {
            auto &secondAtom = secondMolecule->atomStates[j];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            auto totalRadius = firstAtom.radius + secondAtom.radius;
            if(deltaLength2 < totalRadius*totalRadius)
            {
                emitContactPoint(firstMolecule, secondMolecule, i, j);
            }
        }
    }
}


void ContactPoint::computeNormalAndPenetrationDistance()
{
    auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
    auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];

    auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
    auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondMolecule->atomStates[secondAtomIndex].position);

    auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
    auto totalRadius = firstAtom.radius + secondAtom.radius;

    normal = deltaVector.normalized();
    penetrationDistance = totalRadius - deltaVector.length();
    firstRelativePoint = firstAtomWorldPosition - normal*firstAtom.radius - firstMolecule->transform.translation;
    secondRelativePoint = secondAtomWorldPosition + normal*secondAtom.radius - secondMolecule->transform.translation;

    //printf("N %f %f %f - D %f R %f\n", normal.x, normal.y, normal.z, penetrationDistance, totalRadius);
    //printf("First point: %f %f %f\n", firstRelativePoint.x, firstRelativePoint.y, firstRelativePoint.z);
    //printf("Second point: %f %f %f\n", secondRelativePoint.x, secondRelativePoint.y, secondRelativePoint.z);
}

Scalar ContactPoint::computeInverseInertia()
{
    return computeInverseLinearInertia() + computeInverseAngularInertia();
}

Scalar ContactPoint::computeInverseLinearInertia()
{
    return firstMolecule->inverseTotalMass + secondMolecule->inverseTotalMass;
}

Scalar ContactPoint::computeInverseAngularInertia()
{
    return firstMolecule->computeAngularInertiaForRelativeContactPoint(firstRelativePoint, normal)
        + secondMolecule->computeAngularInertiaForRelativeContactPoint(secondRelativePoint, -normal);
}

Matrix3x3 ContactPoint::computeContactSpaceMatrix() const
{
    auto x = normal;
    auto y = Vector3(1, 0, 0);
    if(abs(normal.x) > abs(normal.y))
        y = Vector3(0, 1, 0);

    auto z = x.cross(y).normalized();
    y = z.cross(x).normalized();
    
    return Matrix3x3::WithColumns(x, y, z);
}

inline void Simulation::emitContactPoint(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule, size_t firstAtomIndex, size_t secondAtomIndex)
{

    auto contactPoint = ContactPoint();
    contactPoint.firstMolecule = firstMolecule.get();
    contactPoint.secondMolecule = secondMolecule.get();
    contactPoint.firstAtomIndex = firstAtomIndex;
    contactPoint.secondMolecule = secondMolecule.get();
    contactPoint.secondAtomIndex = secondAtomIndex;
    contactPoint.computeNormalAndPenetrationDistance();
    contactPoints.push_back(contactPoint);
}

void Simulation::resolveContactManifoldsCollisionsAndConstraints()
{
    const int IterationCount = 5;
    for(int i = 0; i < IterationCount; ++i)
    {
        // Collision responses
        for(auto &contact : contactPoints)
            resolveContactCollisionResponse(contact);

        // Collision constraints
        for(auto &contact : contactPoints)
            resolveContactConstraint(contact, 1);
    }
}

void Simulation::resolveContactCollisionResponse(ContactPoint &contact)
{
    contact.computeNormalAndPenetrationDistance();

	// See Milling. 'Game Physics Engine Development'. Chapter 14 for details on these equations and the associated algorithms.	
	auto firstCollisionObject = contact.firstMolecule;
	auto secondCollisionObject = contact.secondMolecule;
	
	auto contactNormal = contact.normal;

	auto relativeFirstPoint = contact.firstRelativePoint;
	auto relativeSecondPoint = contact.secondRelativePoint;

	auto contactLocalToWorldMatrix3x3 = contact.computeContactSpaceMatrix();

    auto velocityChangePerImpulseWorldMatrix = 
        firstCollisionObject->computeVelocityPerImpulseWorldMatrixForRelativeContactPoint(relativeFirstPoint) +
        secondCollisionObject->computeVelocityPerImpulseWorldMatrixForRelativeContactPoint(relativeSecondPoint);
	
	auto velocityChangePerImpulseContactMatrix = contactLocalToWorldMatrix3x3.transposed() * velocityChangePerImpulseWorldMatrix * contactLocalToWorldMatrix3x3;

    auto inverseMass = firstCollisionObject->inverseTotalMass + secondCollisionObject->inverseTotalMass;
	velocityChangePerImpulseContactMatrix = velocityChangePerImpulseContactMatrix + Matrix3x3(inverseMass);
	
    //printf("velocityChangePerImpulseContactMatrix.determinant() %f\n", velocityChangePerImpulseContactMatrix.determinant());
    if (velocityChangePerImpulseContactMatrix.determinant() == 0)
    {
        return;
    }

	auto impulseChangePerVelocityContactMatrix = velocityChangePerImpulseContactMatrix.inverse();

    auto firstContactVelocity = firstCollisionObject->computeVelocityAtRelativePoint(relativeFirstPoint);
	auto secondContactVelocity = secondCollisionObject->computeVelocityAtRelativePoint(relativeSecondPoint);
	
    auto relativeSeparationVelocity = firstContactVelocity - secondContactVelocity;
	
    auto relativeContactSeparationVelocity = relativeSeparationVelocity * contactLocalToWorldMatrix3x3;
    //printf("relativeContactSeparationVelocity.x %f\n", relativeContactSeparationVelocity.x);
	if(relativeContactSeparationVelocity.x > 0)
    {
        return;
    }

	auto relativeVelocityFromIntegrationDelta = firstCollisionObject->linearVelocityIntegrationDelta - secondCollisionObject->linearVelocityIntegrationDelta;
	auto relativeContactVelocityFromIntegrationDelta = relativeVelocityFromIntegrationDelta.dot(contactNormal);
	
	auto restitutionCoefficient = sqrt(firstCollisionObject->restitutionCoefficient * secondCollisionObject->restitutionCoefficient);
	
	// Resting contact: reduce contact velocity by acceleration only speed increase, and set the restitution coefficient to 0.
	if(abs(relativeContactSeparationVelocity.x) < restingContactVelocityLimit)
    { 
		restitutionCoefficient = 0;
    }

	auto deltaVelocity = -relativeContactSeparationVelocity.x - (restitutionCoefficient * (relativeContactSeparationVelocity.x - relativeContactVelocityFromIntegrationDelta));
	
	auto contactLocalVelocityChange = Vector3(
        deltaVelocity,
		-relativeContactSeparationVelocity.y,
		-relativeContactSeparationVelocity.z
    );
		
	auto contactLocalImpulse = impulseChangePerVelocityContactMatrix * contactLocalVelocityChange;

	// Compute the planar length for simulating friction.
	auto staticFrictionCoefficient = std::min(firstCollisionObject->staticFrictionCoefficient, secondCollisionObject->staticFrictionCoefficient);
	auto planarImpulse = sqrt(contactLocalImpulse.y*contactLocalImpulse.y + contactLocalImpulse.z*contactLocalImpulse.z);

	// Is this in the limits for the static friction?
	if(planarImpulse > contactLocalImpulse.x * staticFrictionCoefficient)
    {
        auto dynamicFrictionCoefficient = std::min(firstCollisionObject->dynamicFrictionCoefficient, secondCollisionObject->dynamicFrictionCoefficient);
		contactLocalImpulse.y /= planarImpulse;
		contactLocalImpulse.z /= planarImpulse;

		//contactLocalImpulse yz length = dynamicFrictionCoefficient * contactLocalImpulse x
		
		// CHECK ME: What is the meaning of this correction? [From Millington Game Physics Engine Development, Chapter 15 pp 410]
		//auto frictionNormalDelta = velocityChangePerImpulseContactMatrix.firstRow().dot(Math::Vector3(1, dynamicFrictionCoefficient*contactLocalImpulse.y, dynamicFrictionCoefficient*contactLocalImpulse.z));
        //contactLocalImpulse.x = deltaVelocity / frictionNormalDelta;

		contactLocalImpulse.y *= dynamicFrictionCoefficient * contactLocalImpulse.x;
		contactLocalImpulse.z *= dynamicFrictionCoefficient * contactLocalImpulse.x;
	}

	auto contactImpulse = contactLocalToWorldMatrix3x3 * contactLocalImpulse;

    firstCollisionObject->applyImpulseInRelativePosition(contactImpulse, relativeFirstPoint);
    secondCollisionObject->applyImpulseInRelativePosition(-contactImpulse, relativeSecondPoint);
}

void Simulation::resolveContactConstraint(ContactPoint &contact, Scalar relaxationFactor)
{
    contact.computeNormalAndPenetrationDistance();
    if(contact.penetrationDistance <= 0)
        return;

    auto inverseInertia = contact.computeInverseInertia();
    if(inverseInertia <= 0)
        return;

    auto penetrationDelta = contact.penetrationDistance*relaxationFactor/inverseInertia;

    //printf("N: %f %f %f D: %f\n", contact.normal.x, contact.normal.y, contact.normal.z, contact.penetrationDistance);

    contact.firstMolecule->applyMovementAtRelativePoint(penetrationDelta, contact.firstRelativePoint, contact.normal);
    contact.secondMolecule->applyMovementAtRelativePoint(penetrationDelta, contact.secondRelativePoint, -contact.normal);
}


void Simulation::update(float deltaTime)
{
    resetNetForces();
    evaluateForceGenerators(deltaTime);
    integrateMovement(deltaTime);
    detectAndResolveCollisions();
}

} // End of namespace Molesim
