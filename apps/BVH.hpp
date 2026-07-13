#ifndef MOLESIM_BVH_HPP
#define MOLESIM_BVH_HPP

#include "AABox.hpp"
#include <stdint.h>
#include <algorithm>
#include <memory>
#include <functional>
#include <vector>

namespace Molesim
{
/**
 * Implementation from https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/ [January 2026]
 */
inline uint64_t morton_splitBy3(unsigned int a){
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff; // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff; // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}

/**
 * Morton encoding from: https://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/ [January 2026]
 */
inline uint64_t mortonEncode(unsigned int x, unsigned int y, unsigned int z){
    uint64_t answer = 0;
    answer |= morton_splitBy3(x) | morton_splitBy3(y) << 1 | morton_splitBy3(z) << 2;
    return answer;
}

template<typename PT>
struct BoundingVolumeHierarchyNode
{
    bool isLeaf = false;
    AABox volume;
    std::shared_ptr<BoundingVolumeHierarchyNode<PT>> leftChild;
    std::shared_ptr<BoundingVolumeHierarchyNode<PT>> rightChild;
    
    PT payload;
    uint64_t mortonCode = 0;
    
    template<typename FT>
    void leavesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        if(!volume.hasIntersectionWithBox(box))
            return;
        
        if(isLeaf)
        {
            return aBlock(payload);
        }
        else
        {
            leftChild->leavesIntersectingBoxDo(box, aBlock);
            rightChild->leavesIntersectingBoxDo(box, aBlock);
        }
    }

    template<typename FT>
    void leavesIntersectingRayDo(const Ray3D &ray, FT &&aBlock)
    {
        if(!volume.hasIntersectionWithRay(ray))
            return;
        
        if(isLeaf)
        {
            return aBlock(payload);
        }
        else
        {
            leftChild->leavesIntersectingRayDo(ray, aBlock);
            rightChild->leavesIntersectingRayDo(ray, aBlock);
        }
    }
};

template<typename PT>
class BoundingVolumeHierachy
{
public:
    typedef PT PayloadType;
    typedef BoundingVolumeHierarchyNode<PT> NodeType;
    typedef std::shared_ptr<BoundingVolumeHierarchyNode<PT>> NodePtrType;

    NodePtrType rootNode;

    template<typename FT>
    void leavesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        if(!rootNode)
            return;

        rootNode->leavesIntersectingBoxDo(box, aBlock);
    }

    template<typename FT>
    void leavesIntersectingRayDo(const Ray3D &ray, FT &&aBlock)
    {
        if(!rootNode)
            return;

        rootNode->leavesIntersectingRayDo(ray, aBlock);
    }

    void buildBottomUp(std::vector<NodePtrType> bvhLeaves)
    {
        AABox treeBox = AABox::Empty();
        for(auto &leaf : bvhLeaves)
            treeBox.insertBox(leaf->volume);
        
        auto bitRange = (1<<21) - 1;
        auto extentReciprocal = treeBox.extent().reciprocal();
        for(auto &leaf : bvhLeaves)
        {
            auto center = leaf->volume.center();
            auto centerNormalized = (center - treeBox.minCorner)*extentReciprocal;

            //printf("centerNormalized: %f %f %f\n", centerNormalized.x, centerNormalized.y, centerNormalized.z);
            unsigned int bitX = (unsigned int)(centerNormalized.x*bitRange);
            unsigned int bitY = (unsigned int)(centerNormalized.y*bitRange);
            unsigned int bitZ = (unsigned int)(centerNormalized.z*bitRange);

            //printf("bit: %d %d %d\n", bitX, bitY, bitZ);
            leaf->mortonCode = mortonEncode(bitX, bitY, bitZ);

            //printf("morton code: %lu\n", leaf->mortonCode);
        }
        
        std::sort(bvhLeaves.begin(), bvhLeaves.end(), [](const NodePtrType &a, const NodePtrType &b){
            return a->mortonCode < b->mortonCode;
        });

        constructBottomUpTopology(bvhLeaves);
    }

    void constructBottomUpTopology(const std::vector<NodePtrType> &leaves)
    {
        std::vector<NodePtrType> nodes;
        nodes.reserve(leaves.size()*2);
        for(auto leaf : leaves)
            nodes.push_back(leaf);

        size_t sourceIndex = 0;
        while(sourceIndex + 1 < nodes.size())
        {
            size_t leftNodeIndex = sourceIndex;
            size_t rightNodeIndex = sourceIndex + 1;
            auto &leftNode = nodes[leftNodeIndex];
            auto &rightNode = nodes[rightNodeIndex];

            auto volume = leftNode->volume;
            volume.insertBox(rightNode->volume);

            auto innerNode = std::make_shared<NodeType> ();
            innerNode->volume = volume;
            innerNode->leftChild = leftNode;
            innerNode->rightChild = rightNode;

            nodes.push_back(innerNode);
            sourceIndex += 2;
        }

        if(!nodes.empty())
            rootNode = nodes.back();
    }

};

} // End of namespace Molesim

#endif // MOLESIM_BVH_HPP
