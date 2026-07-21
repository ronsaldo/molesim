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
        if(entries.size() == 0)
        {
            return nullptr;
        }
        else if(entries.size() == 1)
        {
            auto leaf = std::make_shared<TreeNodeType> ();
            leaf->entry = entries[0];
            return leaf;
        }

        int axis = depth % 3;
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
        node->left = leftNode;
        node->right = rightNode;
        return node;
    }

    std::shared_ptr<TreeNodeType> rootNode;
};

} // End of namespace Molesim

#endif //MOLESIM_KD_TREE_HPP
