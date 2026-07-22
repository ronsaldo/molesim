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

// Code taken from UDock2 [http://udock.fr/]
Scalar lennardJonesCoulombic(const Vector3 & p1,
                             Scalar             r1,
                             Scalar             e1,
                             Scalar             c1,
                             const Vector3 & p2,
                             Scalar             r2,
                             Scalar             e2,
                             Scalar             c2 )
{
    const float d = (p2 - p1).length();

    // Avoid 0 division
    if ( d < 1e-1f )
        return std::numeric_limits<float>::max();

    // 8-6 Lennard-Jones potential
    // http://www.sklogwiki.org/SklogWiki/index.php/8-6_Lennard-Jones_potential
    const Scalar rmOutD  = ( r1 + r2 ) / d;
    Scalar       rmOutD6 = rmOutD * rmOutD * rmOutD;
    rmOutD6             = rmOutD6 * rmOutD6;

    const Scalar rmOutD8 = rmOutD6 * rmOutD * rmOutD;
    const Scalar epsilon = sqrt( e1 * e2 );
    const Scalar contact = 3.f * epsilon * rmOutD8 - 4.f * epsilon * rmOutD6;

    // Coulombic
    // "f is a conversion factor for converting the electrostatics term to kcal/mol. We used f = 332.0522
    // according to the AMBER12 documentation. We used a distance dependent dielectric constant of \mathcal{E}_0 =
    // 20 that resulted in balanced contributions of t different terms of the scoring function." Udock, the
    // interactive docking entertainment system, Levieux et al. 2014 Reference:
    // https://cedric.cnam.fr/fichiers/art_3149.pdf
    const Scalar charge = ( 332.0522f / 20.f ) * ( c1 * c2 / d );

    return contact + charge;
}

// Reference: http://chemyang.ccnu.edu.cn/ccb/server/AIMMS/mol2.pdf, p. 53
// Code taken from UDock2 [http://udock.fr/]
std::string sybyl_toString( Sybyl type )
{
    switch ( type )
    {
    case Sybyl::CarbonSp3: return "C.3";
    case Sybyl::CarbonSp2: return "C.2";
    case Sybyl::CarbonAromatic: return "C.ar";
    case Sybyl::Carbocation: return "C.cat";
    case Sybyl::NitrogenSp3: return "N.3";
    case Sybyl::NitrogenSp2: return "N.2";
    case Sybyl::NitrogenSp3PositivelyCharged: return "N.4";
    case Sybyl::NitrogenAromatic: return "N.ar";
    case Sybyl::NitrogenAmide: return "N.am";
    case Sybyl::NitrogenTrigonalPlanar: return "N.pl3";
    case Sybyl::OxygenSp3: return "O.3";
    case Sybyl::OxygenSp2: return "O.2";
    case Sybyl::OxygenInCarboxylatesAndPhosphates: return "O.co2";
    case Sybyl::SulphurSp3: return "S.3";
    case Sybyl::PhosphorusSp3: return "P.3";
    case Sybyl::Fluorine: return "F";
    case Sybyl::Hydrogen: return "H";
    case Sybyl::Lithium: return "Li";
    }

    return "";
}

// Code taken from UDock2 [http://udock.fr/]
Sybyl sybyl_fromString( const std::string &type )
{
    if ( type == "C.3" )
        return Sybyl::CarbonSp3;
    if ( type == "C.2" )
        return Sybyl::CarbonSp2;
    if ( type == "C.ar" )
        return Sybyl::CarbonAromatic;
    if ( type == "C.cat" )
        return Sybyl::Carbocation;
    if ( type == "N.3" )
        return Sybyl::NitrogenSp3;
    if ( type == "N.2" )
        return Sybyl::NitrogenSp2;
    if ( type == "N.4" )
        return Sybyl::NitrogenSp3PositivelyCharged;
    if ( type == "N.ar" )
        return Sybyl::NitrogenAromatic;
    if ( type == "N.am" )
        return Sybyl::NitrogenAmide;
    if ( type == "N.pl3" )
        return Sybyl::NitrogenTrigonalPlanar;
    if ( type == "O.3" )
        return Sybyl::OxygenSp3;
    if ( type == "O.2" )
        return Sybyl::OxygenSp2;
    if ( type == "O.co2" )
        return Sybyl::OxygenInCarboxylatesAndPhosphates;
    if ( type == "S.3" )
        return Sybyl::SulphurSp3;
    if ( type == "F" )
        return Sybyl::Fluorine;
    if ( type == "H" )
        return Sybyl::Hydrogen;
    if ( type == "Li" )
        return Sybyl::Lithium;

    return Sybyl::Hydrogen;
}

// Code taken from UDock2 [http://udock.fr/]
float sybyl_getRadius( Sybyl type )
{
    switch ( type )
    {
    case Sybyl::CarbonSp3: return 1.908f;
    case Sybyl::CarbonSp2: return 1.908f;
    case Sybyl::CarbonAromatic: return 1.908f;
    case Sybyl::Carbocation: return 1.908f;
    case Sybyl::NitrogenSp3: return 1.875f;
    case Sybyl::NitrogenSp2: return 1.874f;
    case Sybyl::NitrogenSp3PositivelyCharged: return 1.824f;
    case Sybyl::NitrogenAromatic: return 1.824f;
    case Sybyl::NitrogenAmide: return 1.824f;
    case Sybyl::NitrogenTrigonalPlanar: return 1.824f;
    case Sybyl::OxygenSp3: return 1.721f;
    case Sybyl::OxygenSp2: return 1.6612f;
    case Sybyl::OxygenInCarboxylatesAndPhosphates: return 1.6612f;
    case Sybyl::SulphurSp3: return 2.0f;
    case Sybyl::PhosphorusSp3: return 2.1f;
    case Sybyl::Fluorine: return 1.75f;
    case Sybyl::Hydrogen: return 1.4870f;
    case Sybyl::Lithium: return 1.137f;
    }

    return 0.f;
}

// Cornell, W. D., Cieplak, P., Bayly, C. I., Gould, I. R., Merz, K. M., Ferguson, D. M., Spellmeyer, D. C., Fox,
// T., Caldwell, J. W., & Kollman, P. A. (1995). A Second Generation Force Field for the Simulation of Proteins,
// Nucleic Acids, and Organic Molecules. In Journal of the American Chemical Society (Vol. 117, Issue 19, pp.
// 5179�5197). American Chemical Society (ACS). https://doi.org/10.1021/ja00124a002
// Code taken from UDock2 [http://udock.fr/]
float sybyl_getEpsilon( Sybyl type )
{
    switch ( type )
    {
    case Sybyl::CarbonSp3: return 0.1094f;
    case Sybyl::CarbonSp2: return 0.0860f;
    case Sybyl::CarbonAromatic: return 0.0860f;
    case Sybyl::Carbocation: return 0.0860f;
    case Sybyl::NitrogenSp3: return 0.17f;
    case Sybyl::NitrogenSp2: return 0.17f;
    case Sybyl::NitrogenSp3PositivelyCharged: return 0.17f;
    case Sybyl::NitrogenAromatic: return 0.17f;
    case Sybyl::NitrogenAmide: return 0.17f;
    case Sybyl::NitrogenTrigonalPlanar: return 0.17f;
    case Sybyl::OxygenSp3: return 0.2104f;
    case Sybyl::OxygenSp2: return 0.21f;
    case Sybyl::OxygenInCarboxylatesAndPhosphates: return 0.21f;
    case Sybyl::SulphurSp3: return 0.25f;
    case Sybyl::PhosphorusSp3: return 0.2f;
    case Sybyl::Fluorine: return 0.061f;
    case Sybyl::Hydrogen: return 0.0157f;
    case Sybyl::Lithium: return 0.0183f;
    }

    return 0.f;
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

        std::string sybylName = chemAtom.get("sybyl").value_or("").as_string();
        auto sybylType = sybyl_fromString(sybylName);

        auto atomDesc = AtomDescription();
        atomDesc.charge = float(chemAtom.charge());
        atomDesc.epsilon = sybyl_getEpsilon(sybylType);
        atomDesc.mass = chemAtom.mass();
        atomDesc.atomicNumber = chemAtomNumber ? chemAtomNumber.value() : 1;

        //printf("Atom %zu charge %f eps: %f mass: %f sybyl [%s]%d\n", i, atomDesc.charge, atomDesc.epsilon, atomDesc.mass, sybylName.c_str(), int(sybylType));

        auto atomState = AtomRenderingState();
        atomState.position = Vector3(atomPosition[0], atomPosition[1], atomPosition[2]);
        //printf("Position: %f %f %f\n", atomPosition[0], atomPosition[1], atomPosition[2]);
        
        atomState.radius = sybyl_getRadius(sybylType);
        atomState.color = getOrCreateColorForAtomType(chemAtom.type());
        
        molecule->atomStates.push_back(atomState);
        molecule->atomDescriptions.push_back(atomDesc);
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

void Molecule::computeBVH()
{
    std::vector<MoleculeBVH::NodePtrType> bvhLeaves;
    bvhLeaves.reserve(atomStates.size());

    for(size_t i = 0; i < atomStates.size(); ++i)
    {
        auto center = atomStates[i].position;
        auto radius = atomStates[i].radius;
        auto volume = AABox::ForSphere(center, radius);

        auto leaf = std::make_shared<MoleculeBVH::NodeType> ();
        leaf->isLeaf = true;
        leaf->volume = volume;
        leaf->payload = i;
        bvhLeaves.push_back(leaf);
    }

    bvh.buildBottomUp(bvhLeaves);
}

void Molecule::computeGrid()
{
    grid.setupForBoundingBox(boundingBox);
    for(size_t i = 0; i < atomStates.size(); ++i)
    {
        auto center = atomStates[i].position;
        grid.addPoint(center, i);
    }
}

void Molecule::computeOctree()
{
    std::vector<MoleculeOctree::EntryType> entries;
    for(size_t i = 0; i < atomStates.size(); ++i)
    {
        auto center = atomStates[i].position;
        auto entry = MoleculeOctree::EntryType(center, i);
        entries.push_back(entry);
    }

    octree.setupForBoundingBox(boundingBox);
    octree.addEntries(entries);
}

void Molecule::computeKDTree()
{
    std::vector<MoleculeKDTree::EntryType> entries;
    for(size_t i = 0; i < atomStates.size(); ++i)
    {
        auto center = atomStates[i].position;
        auto entry = MoleculeKDTree::EntryType(center, i);
        entries.push_back(entry);
    }

    kdTree.buildWithEntries(entries);
}

void Molecule::prepareForSimulation(SpatialSubdivisionAlgorithm spatialSubdivisionAlgorithm)
{
    translateToCenterOfMass();
    computeBoundingBox();
    computeInertiaTensor();
    switch (spatialSubdivisionAlgorithm)
    {
    case SpatialSubdivisionAlgorithm::Naive:
        // Nothing is required here
        break;
    case SpatialSubdivisionAlgorithm::Grid:
        computeGrid();
        break;
    case SpatialSubdivisionAlgorithm::KDTree:
        computeKDTree();
        break;
    case SpatialSubdivisionAlgorithm::Octree:
        computeOctree();
        break;
    case SpatialSubdivisionAlgorithm::BVH:
        computeBVH();
        break;
    }
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

void Molecule::createFirstTestMolecule(SpatialSubdivisionAlgorithm spatialSubdivisionAlgorithm)
{
    {
        AtomDescription atomDescription;
        atomDescription.mass = 1.0;
        atomDescription.charge = 1.0;
        atomDescription.epsilon = 1.0;
        atomDescriptions.push_back(atomDescription);

        AtomRenderingState atomState;
        atomState.position = Vector3();
        atomState.radius = 1.0;
        atomState.color = Vector4(0.8f, 0.1f, 0.1f, 1.0f);
        atomStates.push_back(atomState);
    }
    prepareForSimulation(spatialSubdivisionAlgorithm);
    transform.translation = Vector3(-0.25, 0.0, 0.0);
}

void Molecule::createSecondTestMolecule(SpatialSubdivisionAlgorithm spatialSubdivisionAlgorithm)
{
    {
        AtomDescription atomDescription;
        atomDescription.mass = 1.0;
        atomDescription.charge = 1.0;
        atomDescription.epsilon = 1.0;
        atomDescriptions.push_back(atomDescription);

        AtomRenderingState atomState;
        atomState.position = Vector3();
        atomState.radius = 1.0;
        atomState.color = Vector4(0.8f, 0.1f, 0.1f, 1.0f);
        atomStates.push_back(atomState);
    }
    prepareForSimulation(spatialSubdivisionAlgorithm);
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

Simulation::Simulation()
{
    randomEngine.seed(42);
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

Scalar Simulation::computeTotalEnergy()
{
    auto broadphasePairs = computeBroadphase();
    return computeTotalEnergyWithPairs(broadphasePairs);
}

std::vector<std::pair<MoleculePtr, MoleculePtr>> Simulation::computeBroadphase()
{
    std::vector<std::pair<MoleculePtr, MoleculePtr>> broadphasePairs;
    for(size_t i = 0; i < molecules.size(); ++i)
    {
        auto &firstMolecule = molecules[i];
        auto firstBoundingBox = firstMolecule->boundingBox.transformedWith(firstMolecule->transform).expandedBy(energyMaxRadiusDefault);
        for(size_t j = i + 1; j < molecules.size(); ++j)
        {
            auto &secondMolecule = molecules[j];
            auto secondBoundingBox = secondMolecule->boundingBox.transformedWith(secondMolecule->transform).expandedBy(energyMaxRadiusDefault);
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
    switch (spatialSubdivisionAlgorithm)
    {
    case SpatialSubdivisionAlgorithm::Naive:
        computeNaivePairNarrowPhase(firstMolecule, secondMolecule);
        break;
    case SpatialSubdivisionAlgorithm::Grid:
        computeGridPairNarrowPhase(firstMolecule, secondMolecule);
        break;
    case SpatialSubdivisionAlgorithm::KDTree:
        computeKDTreePairNarrowPhase(firstMolecule, secondMolecule);
        break;
    case SpatialSubdivisionAlgorithm::Octree:
        computeOctreePairNarrowPhase(firstMolecule, secondMolecule);
        break;
    case SpatialSubdivisionAlgorithm::BVH:
        computeBVHPairNarrowPhase(firstMolecule, secondMolecule);
        break;
    default:
        abort();
        break;
    }
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
                emitContactPoint(firstMolecule, secondMolecule, i, j);
        }
    }
}

void Simulation::computeGridPairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius);

        secondMolecule->grid.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            auto totalRadius = firstAtom.radius + secondAtom.radius;
            if(deltaLength2 < totalRadius*totalRadius)
                emitContactPoint(firstMolecule, secondMolecule, firstAtomIndex, secondAtomIndex);
        });
    }
}

void Simulation::computeKDTreePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius);

        secondMolecule->kdTree.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            auto totalRadius = firstAtom.radius + secondAtom.radius;
            if(deltaLength2 < totalRadius*totalRadius)
                emitContactPoint(firstMolecule, secondMolecule, firstAtomIndex, secondAtomIndex);
        });
    }
}

void Simulation::computeOctreePairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius);

        secondMolecule->octree.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            auto totalRadius = firstAtom.radius + secondAtom.radius;
            if(deltaLength2 < totalRadius*totalRadius)
                emitContactPoint(firstMolecule, secondMolecule, firstAtomIndex, secondAtomIndex);
        });
    }
}

void Simulation::computeBVHPairNarrowPhase(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius);

        secondMolecule->bvh.leavesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            auto totalRadius = firstAtom.radius + secondAtom.radius;
            if(deltaLength2 < totalRadius*totalRadius)
                emitContactPoint(firstMolecule, secondMolecule, firstAtomIndex, secondAtomIndex);
        });
    }
}

Scalar Simulation::computeTotalEnergyWithPairs(const std::vector<std::pair<MoleculePtr, MoleculePtr>> &broadphasePairs)
{
    Scalar totalEnergy = 0;
    for(auto &pair: broadphasePairs)
        totalEnergy += computePairEnergy(pair.first, pair.second);

    return totalEnergy;
}

Scalar Simulation::computePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    switch (spatialSubdivisionAlgorithm)
    {
    case SpatialSubdivisionAlgorithm::Naive:
        return computeNaivePairEnergy(firstMolecule, secondMolecule);
    case SpatialSubdivisionAlgorithm::Grid:
        return computeGridPairEnergy(firstMolecule, secondMolecule);
    case SpatialSubdivisionAlgorithm::KDTree:
        return computeKDTreePairEnergy(firstMolecule, secondMolecule);
    case SpatialSubdivisionAlgorithm::Octree:
        return computeOctreePairEnergy(firstMolecule, secondMolecule);
    case SpatialSubdivisionAlgorithm::BVH:
        return computeBVHPairEnergy(firstMolecule, secondMolecule);
    default:
        abort();
        break;
    }
}

Scalar Simulation::computeNaivePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    Scalar energy = 0;
    for(size_t i = 0; i < firstMolecule->atomStates.size(); ++i)
    {
        auto &firstAtom = firstMolecule->atomStates[i];
        auto &firstAtomDesc = firstMolecule->atomDescriptions[i];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        for(size_t j = 0; j < secondMolecule->atomStates.size(); ++j)
        {
            auto &secondAtom = secondMolecule->atomStates[j];
            auto &secondAtomDesc = secondMolecule->atomDescriptions[j];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            if(deltaLength2 > energyMaxRadiusDefault*energyMaxRadiusDefault)
                continue;

            energy += lennardJonesCoulombic(
                    firstAtomWorldPosition, firstAtom.radius, firstAtomDesc.epsilon, firstAtomDesc.charge,
                    secondAtomWorldPosition, secondAtom.radius, secondAtomDesc.epsilon, secondAtomDesc.charge);
        }
    }

    return energy;
}

Scalar Simulation::computeGridPairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    Scalar energy = 0;
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto &firstAtomDesc = firstMolecule->atomDescriptions[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius + energyMaxRadiusDefault);

        secondMolecule->grid.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto &secondAtomDesc = secondMolecule->atomDescriptions[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            if(deltaLength2 > energyMaxRadiusDefault*energyMaxRadiusDefault)
                return;

            energy += lennardJonesCoulombic(
                    firstAtomWorldPosition, firstAtom.radius, firstAtomDesc.epsilon, firstAtomDesc.charge,
                    secondAtomWorldPosition, secondAtom.radius, secondAtomDesc.epsilon, secondAtomDesc.charge);
        });
    }

    return energy;
}

Scalar Simulation::computeKDTreePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    Scalar energy = 0;
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto &firstAtomDesc = firstMolecule->atomDescriptions[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius + energyMaxRadiusDefault);

        secondMolecule->kdTree.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto &secondAtomDesc = secondMolecule->atomDescriptions[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            if(deltaLength2 > energyMaxRadiusDefault*energyMaxRadiusDefault)
                return;

            energy += lennardJonesCoulombic(
                    firstAtomWorldPosition, firstAtom.radius, firstAtomDesc.epsilon, firstAtomDesc.charge,
                    secondAtomWorldPosition, secondAtom.radius, secondAtomDesc.epsilon, secondAtomDesc.charge);
        });
    }

    return energy;
}

Scalar Simulation::computeOctreePairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    Scalar energy = 0;
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto &firstAtomDesc = firstMolecule->atomDescriptions[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius + energyMaxRadiusDefault);

        secondMolecule->octree.nodesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto &secondAtomDesc = secondMolecule->atomDescriptions[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            if(deltaLength2 > energyMaxRadiusDefault*energyMaxRadiusDefault)
                return;

            energy += lennardJonesCoulombic(
                    firstAtomWorldPosition, firstAtom.radius, firstAtomDesc.epsilon, firstAtomDesc.charge,
                    secondAtomWorldPosition, secondAtom.radius, secondAtomDesc.epsilon, secondAtomDesc.charge);
        });
    }

    return energy;
}

Scalar Simulation::computeBVHPairEnergy(const MoleculePtr &firstMolecule, const MoleculePtr &secondMolecule)
{
    Scalar energy = 0;
    for(size_t firstAtomIndex = 0; firstAtomIndex < firstMolecule->atomStates.size(); ++firstAtomIndex)
    {
        auto &firstAtom = firstMolecule->atomStates[firstAtomIndex];
        auto &firstAtomDesc = firstMolecule->atomDescriptions[firstAtomIndex];
        auto firstAtomWorldPosition = firstMolecule->transform.transformPosition(firstAtom.position);
        auto firstAtomPositionInSecondMolecule = secondMolecule->transform.inverseTransformPosition(firstAtomWorldPosition);
        auto firstAtomBoundingBoxInSecondMolecule = AABox::ForSphere(firstAtomPositionInSecondMolecule, firstAtom.radius + energyMaxRadiusDefault);

        secondMolecule->bvh.leavesIntersectingBoxDo(firstAtomBoundingBoxInSecondMolecule, [&](size_t secondAtomIndex){
            auto &secondAtom = secondMolecule->atomStates[secondAtomIndex];
            auto &secondAtomDesc = secondMolecule->atomDescriptions[secondAtomIndex];
            auto secondAtomWorldPosition = secondMolecule->transform.transformPosition(secondAtom.position);

            auto deltaVector = firstAtomWorldPosition - secondAtomWorldPosition;
            auto deltaLength2 = deltaVector.length2();
            if(deltaLength2 > energyMaxRadiusDefault*energyMaxRadiusDefault)
                return;

            energy += lennardJonesCoulombic(
                    firstAtomWorldPosition, firstAtom.radius, firstAtomDesc.epsilon, firstAtomDesc.charge,
                    secondAtomWorldPosition, secondAtom.radius, secondAtomDesc.epsilon, secondAtomDesc.charge);
        });
    }

    return energy;
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

void Simulation::performOptimizationSteps()
{
    if(simulationMoleculeIndex >= molecules.size())
        return;

    performOptimizationLoadTransform();
    for(size_t optimizationStep = 0; optimizationStep < optimizationStepCount; ++optimizationStep)
        performOptimizationStep();
}

void Simulation::performOptimizationLoadTransform()
{
    auto &simulationMolecule = molecules[simulationMoleculeIndex];

    Scalar energy = computeTotalEnergy();
    if(energy < simulationMolecule->bestTransformEnergy)
    {
        simulationMolecule->bestTransform = simulationMolecule->transform;
        simulationMolecule->bestTransformEnergy = energy;
    }
        
}

void Simulation::performOptimizationStep()
{
    auto &simulationMolecule = molecules[simulationMoleculeIndex];

    auto randomVector = Vector3(nextRandomScalar(-1, 1), nextRandomScalar(-1, 1), nextRandomScalar(-1, 1));
    auto randomTransform = RigidTransform::WithTranslation(randomVector);
    simulationMolecule->transform = randomTransform.transformTransform(simulationMolecule->bestTransform);

    Scalar energy = computeTotalEnergy();
    if(energy < simulationMolecule->bestTransformEnergy)
    {
        //printf("Energy %f\n", energy);
        simulationMolecule->bestTransform = simulationMolecule->transform;
        simulationMolecule->bestTransformEnergy = energy;
    }
    else
    {
        simulationMolecule->transform = simulationMolecule->bestTransform;

    }

    //printf("Energy %f randVector %f %f %f\n", energy, randomVector.x, randomVector.y, randomVector.z);
    //simulationMolecule->transform.translation += randomVector;
}

} // End of namespace Molesim
