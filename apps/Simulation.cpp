#include "Simulation.hpp"
#include <chemfiles.hpp>

namespace Molesim
{

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

} // End of namespace Molesim
