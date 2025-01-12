#ifndef OCTREE_NODE_H
#define OCTREE_NODE_H

#include "bounds.h"
#include <array>
#include <glm/glm.hpp>
#include <memory>

template <typename TNode> class OctreeNode
{
  public:
    std::array<TNode*, 8> *_children = nullptr;
    TNode *_parent = nullptr;
    const glm::vec3 _center = {0,0,0}; //center in world coordinates
    const int _size = 0; //size as power of two, e.g. _size = 4, implies => 2^4 = 16

    OctreeNode(TNode *parent, const glm::vec3 &center, int size)
        : _parent(parent), _center(center), _size(size), _children(nullptr)
    {
    }

    ~OctreeNode()
    {
        prune_children();
    }

    // inline int get_size() const
    // {
    //     return 1 << _size;
    // }

    inline bool is_leaf() const
    {
        return _children == nullptr;
    }

    inline float edge_length(const float scale) const
    {
        return (1 << _size) * scale;
    }

    inline void prune_children()
    {
        delete[] _children;
        _children = nullptr;
    }

    inline Bounds get_bounds(const float scale) const
    {
        auto halfEdge = glm::vec3(edge_length(scale) * 0.5f);
        return Bounds(_center - halfEdge, _center + halfEdge);
    }

    void subdivide(const float scale)
    {
        if (_size <= min_size() || !is_leaf())
            return;

        float childOffset = edge_length(scale) * 0.25f;
        int childSize = _size - 1;
        static const glm::vec3 ChildPositions[] = {{-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1},
                                                   {-1, -1, 1},  {1, -1, 1},  {-1, 1, 1},  {1, 1, 1}};

        _children = new std::array<TNode*, 8>();
        for (int i = 0; i < 8; i++)
        {
            (*_children)[i] = create_child_node(_center + ChildPositions[i] * childOffset, childSize);
        }
    }

    int get_count() const
    {
        int count = 1;
        if (!is_leaf())
        {
            for (const auto &child : *_children)
            {
                count += child->get_count();
            }
        }
        return count;
    }

  protected:
    inline virtual int min_size() const
    {
        return 0;
    }
    inline virtual TNode* create_child_node(const glm::vec3 &center, int size) = 0;
};

#endif // OCTREE_NODE_H
