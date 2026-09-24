#ifndef MOLESIM_OCTREE_HPP
#define MOLESIM_OCTREE_HPP

#include "AABox.hpp"
#include <memory>
#include <vector>
#include <assert.h>

namespace Molesim
{

template<typename PT>
struct OctreeEntry
{
    typedef PT PayloadType;
    OctreeEntry() = default;
    OctreeEntry(const AABox &initVolume, const PayloadType &initPayload)
        : volume(initVolume), payload(initPayload) {}

    AABox volume;
    PayloadType payload;
};

template<typename PT>
struct OctreeNode
{
    typedef OctreeNode<PT> NodeType;
    typedef OctreeEntry<PT> EntryType;
    static const int MaxDepth = 16;
    static const size_t SubdivisionThreshold = 8;


    AABox boundingBox;
    Vector3 center;
    int depth = 0;
    std::shared_ptr<OctreeNode> children[8];
    std::vector<EntryType> entries;

    void setBoundingBox(const AABox &newBoundingBox)
    {
        boundingBox = newBoundingBox;
        center = boundingBox.center();
    }

    bool isLeaf() const
    {
        return !children[0];
    }

    void addEntry(const EntryType &entry)
    {
        if(isLeaf())
        {
            entries.push_back(entry);
            subdivideIfNeeded();
            return;
        }
        else
        {
            addEntryToChild(entry);
        }
    }

    void addEntryToChild(const EntryType &entry)
    {
        assert(!isLeaf());

        for(int i = 0; i < 8; ++i)
        {
            auto &child = children[i];
            if(child->boundingBox.hasIntersectionWithBox(entry.volume))
                child->addEntry(entry);
        }
    }

    void subdivideIfNeeded()
    {
        if(!isLeaf())
            return;

        if(depth >= MaxDepth)
            return;
        if(entries.size() <= SubdivisionThreshold)
            return;

        //printf("Subdivide depth: %d\n", depth);

        for(size_t i = 0; i < 8; ++i)
        {
            auto child = std::make_shared<NodeType> ();
            bool isRight = i & 1;
            bool isTop = i & 2;
            bool isFar = i & 4;

            auto childBounds = AABox(
                Vector3(
                    isRight ? center.x : boundingBox.minCorner.x,
                    isTop   ? center.y : boundingBox.minCorner.y,
                    isFar   ? center.z : boundingBox.minCorner.z
                ),
                Vector3(
                    isRight ? boundingBox.maxCorner.x : center.x,
                    isTop   ? boundingBox.maxCorner.y : center.y,
                    isFar   ? boundingBox.maxCorner.z : center.z

                )
            );
            child->setBoundingBox(childBounds);
            child->depth = depth + 1;
            children[i] = child;
        }

        for(auto entry : entries)
            addEntryToChild(entry);
        entries.clear();
    }

    template<typename FT>
    void nodesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        if(!box.hasIntersectionWithBox(boundingBox))
            return;

        if(isLeaf())
        {
            for(auto &entry : entries)
                aBlock(entry.payload);
        }
        else
        {
            for(auto &child : children)
                child->nodesIntersectingBoxDo(box, aBlock);
        }
    }
};

template<typename PT>
struct Octree
{
    typedef OctreeEntry<PT> EntryType;
    typedef OctreeNode<PT> NodeType;

    typedef PT PayloadType;

    void setupForBoundingBox(const AABox &boundingBox)
    {
        rootNode = std::make_shared<NodeType> ();
        rootNode->setBoundingBox(boundingBox);
    }

    void addEntries(const std::vector<EntryType> &entries)
    {
        for(auto &entry : entries)
            addEntry(entry);
    }

    void addEntry(const EntryType &entry)
    {
        rootNode->addEntry(entry);
    }

    template<typename FT>
    void nodesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        std::unordered_set<PayloadType> visitedElements;
        rootNode->nodesIntersectingBoxDo(box, [&](const PayloadType &payload){
            if(visitedElements.find(payload) != visitedElements.end())
                return;

            visitedElements.insert(payload);
            aBlock(payload);
        });
    }

    std::shared_ptr<NodeType> rootNode;
};

} // End of namespace Molesim

#endif //MOLESIM_OCTREE_HPP