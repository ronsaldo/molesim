#ifndef MOLESIM_KD_TREE_HPP
#define MOLESIM_KD_TREE_HPP

#include "Vector3.hpp"
#include <memory>
#include <algorithm>

namespace Molesim
{

template<typename PT>
struct KDTreeEntry
{
    typedef PT PayloadType;
    KDTreeEntry() = default;
    KDTreeEntry(const Vector3 &initPoint, const PayloadType &initPayload)
        : point(initPoint), payload(initPayload) {}

    Vector3 point;
    PayloadType payload;
};


template<typename PT>
struct KDTreeNode
{
    typedef KDTreeEntry<PT> EntryType;
    typedef KDTreeNode<PT> TreeNodeType;
    typedef PT PayloadType;

    EntryType entry;
    size_t axis;
    std::shared_ptr<TreeNodeType> left;
    std::shared_ptr<TreeNodeType> right;
};

template<typename PT>
struct KDTree
{
    typedef KDTreeEntry<PT> EntryType;
    typedef KDTreeNode<PT> TreeNodeType;
    typedef PT PayloadType;

    void buildWithEntries(std::vector<EntryType> entries)
    {
        rootNode = buildNodeWithEntries(entries, 0);
    }

    std::shared_ptr<TreeNodeType> buildNodeWithEntries(std::vector<EntryType> &entries, int depth)
    {
        int axis = depth % 3;
        if(entries.size() == 0)
        {
            return nullptr;
        }
        else if(entries.size() == 1)
        {
            auto leaf = std::make_shared<TreeNodeType> ();
            leaf->entry = entries[0];
            leaf->axis = axis;
            return leaf;
        }

        std::sort(entries.begin(), entries.end(), [=](const EntryType &a, const EntryType &b){
            return a.point.elements[axis] < b.point.elements[axis];
        });
        
        size_t middleIndex = entries.size() / 2;
        auto &middleEntry = entries[middleIndex];

        auto leftEntries = std::vector(entries.begin(), entries.begin() + middleIndex);
        auto rightEntries = std::vector(entries.begin() + middleIndex + 1, entries.end());
        auto leftNode = buildNodeWithEntries(leftEntries, depth + 1);
        auto rightNode = buildNodeWithEntries(rightEntries, depth + 1);
        auto node = std::make_shared<TreeNodeType> ();
        node->entry = middleEntry;
        node->axis = axis;
        node->left = leftNode;
        node->right = rightNode;
        return node;
    }

    template<typename FT>
    void nodeRecursivelyIntersectingBoxDo(TreeNodeType *node, const AABox &box, FT &&aBlock)
    {
        if(!node)
            return;

        // Are we contained in the box?
        if(box.containsPoint(node->entry.point))
            aBlock(node->entry.payload);

        auto axis = node->axis;
        if(box.minCorner.elements[axis] <= node->entry.point.elements[axis])
            nodeRecursivelyIntersectingBoxDo(node->left.get(), box, aBlock);

        if(box.maxCorner.elements[axis] >= node->entry.point.elements[axis])
            nodeRecursivelyIntersectingBoxDo(node->right.get(), box, aBlock);
    }

    template<typename FT>
    void nodesIntersectingBoxDo(const AABox &box, FT &&aBlock)
    {
        nodeRecursivelyIntersectingBoxDo(rootNode.get(), box, aBlock);
    }

    std::shared_ptr<TreeNodeType> rootNode;
};

} // End of namespace Molesim

#endif //MOLESIM_KD_TREE_HPP
