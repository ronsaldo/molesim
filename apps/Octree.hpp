#ifndef MOLESIM_OCTREE_HPP
#define MOLESIM_OCTREE_HPP

#include "AABox.hpp"
#include <memory>
#include <vector>

namespace Molesim
{

template<typename PT>
struct OctreeEntry
{
    typedef PT PayloadType;
    OctreeEntry() = default;
    OctreeEntry(const Vector3 &initPoint, const PayloadType &initPayload)
        : point(initPoint), payload(initPayload) {}

    Vector3 point;
    PayloadType payload;
};

template<typename PT>
struct OctreeNode
{
    typedef OctreeEntry<PT> EntryType;
};

template<typename PT>
struct Octree
{
    typedef OctreeEntry<PT> EntryType;
    typedef OctreeNode<PT> NodeType;

    void setupForBoundingBox(const AABox &boundingBox)
    {
    }

    void addEntries(const std::vector<EntryType> &entries)
    {
    }

    void addEntry(const EntryType &entry)
    {
    }

    template<typename FT>
    void nodesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
    }

    std::shared_ptr<NodeType> rootNode;
};

} // End of namespace Molesim

#endif //MOLESIM_OCTREE_HPP