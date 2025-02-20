#include "voxel_octree_node.h"
#include "jar_voxel_terrain.h"
#include <algorithm>
#include <cmath>
#include <execution>

VoxelOctreeNode::VoxelOctreeNode(int size) : VoxelOctreeNode(nullptr, glm::vec3(0.0f), size)
{
}

VoxelOctreeNode::VoxelOctreeNode(VoxelOctreeNode *parent, const glm::vec3 center, int size)
    : OctreeNode(parent, center, size)
{
    if (_parent != nullptr)
    {
        LoD = _parent->LoD;
        _value = _parent->get_value();
        _isSet = _parent->_isSet;
        NodeColor = _parent->NodeColor;
    }
}

int VoxelOctreeNode::priority() const
{
    return LoD;
}

bool VoxelOctreeNode::is_dirty() const
{
    return _isDirty;
}

void VoxelOctreeNode::set_dirty(bool value)
{
    if (!_isDirty && value && _parent != nullptr)
    {
        _parent->set_dirty(true);
    }
    _isDirty = value;
}

float VoxelOctreeNode::get_value()
{
    if (!is_dirty())
        return _value;
    if (!is_leaf())
    {
        _value = 0;
        // NodeColor = glm::vec4(0, 0, 0, 0);
        for (auto &child : (*_children))
        {
            _value += child->get_value();
            // NodeColor += child->NodeColor;
        }
        _value *= 0.125f;
        // NodeColor *= 0.125f;
    }
    _isDirty = false;
    return _value;
}

void VoxelOctreeNode::set_value(float value)
{
    _value = value;
    _isDirty = false;
    if (_parent != nullptr)
    {
        _parent->set_dirty(true);
    }
}

inline bool VoxelOctreeNode::is_chunk(const JarVoxelTerrain &terrain) const
{
    return _size == (LoD + terrain.get_min_chunk_size());
}

inline bool VoxelOctreeNode::is_one_above_chunk(const JarVoxelTerrain &terrain) const
{
    return _size == (LoD + terrain.get_min_chunk_size() + 1);
}

inline bool VoxelOctreeNode::is_not_edge_chunk(const JarVoxelTerrain &terrain) const
{
    // return true;
    auto e = edge_length(terrain.get_octree_scale());
    return LoD <= terrain.get_lod()->lod_at(_center + glm::vec3(e, 0, 0)) &&
           LoD <= terrain.get_lod()->lod_at(_center - glm::vec3(e, 0, 0)) &&
           LoD <= terrain.get_lod()->lod_at(_center + glm::vec3(0, e, 0)) &&
           LoD <= terrain.get_lod()->lod_at(_center - glm::vec3(0, e, 0)) &&
           LoD <= terrain.get_lod()->lod_at(_center + glm::vec3(0, 0, e)) &&
           LoD <= terrain.get_lod()->lod_at(_center - glm::vec3(0, 0, e));
}

void VoxelOctreeNode::populateUniqueLoDValues(std::vector<int> &lodValues) const
{
    if (std::find(lodValues.begin(), lodValues.end(), LoD) == lodValues.end())
    {
        lodValues.push_back(LoD);
    }
    if (is_leaf())
        return;
    for (const auto &child : *_children)
    {
        child->populateUniqueLoDValues(lodValues);
    }
}

bool VoxelOctreeNode::is_enqueued() const
{
    return _isEnqueued;
}

bool VoxelOctreeNode::is_parent_enqueued() const
{
    return _parent == nullptr ? false : _parent->is_enqueued();
}

bool VoxelOctreeNode::is_any_children_enqueued() const
{
    if (is_leaf())
        return false;
    for (const auto &child : *_children)
    {
        if (child->is_enqueued())
            return true;
    }
    return false;
}

inline bool VoxelOctreeNode::should_delete_chunk(const JarVoxelTerrain &terrain) const
{
    return false;
}

void VoxelOctreeNode::build(JarVoxelTerrain &terrain)
{
    LoD = terrain.get_lod()->desired_lod(*this);

    if (!is_chunk(terrain))
        delete_chunk();

    if (LoD < 0)
        return;

    if (!_isSet)
    {
        float value = terrain.get_sdf()->distance(_center);
        set_value(value);
        if (has_surface(terrain, value) && (_size > LoD))
        {
            subdivide(terrain._octreeScale);
            _isSet = true;
        }
        if (_size == min_size())
            _isSet = true;
    }

    if (!is_leaf() && !(is_chunk(terrain) && (_chunk != nullptr || is_enqueued())))
        for (auto &child : *_children)        
            child->build(terrain);

    if (is_chunk(terrain) && !is_leaf() && (_chunk == nullptr || _chunk->is_edge_chunk() && !is_not_edge_chunk(terrain)))
        queue_update(terrain);
}

bool VoxelOctreeNode::has_surface(const JarVoxelTerrain &terrain, const float value)
{
    return std::abs(value) <
           (1 << _size) * terrain._octreeScale * 1.44224957f * 1.5f; //(3*(1/2)^3)^(1/3) = 1.44224957 for d instead of r
}

void VoxelOctreeNode::modify_sdf_in_bounds(JarVoxelTerrain &terrain, const ModifySettings &settings)
{
    if (settings.sdf.is_null())
    {
        UtilityFunctions::print("sdf invalid");
        return;
    }

    auto bounds = get_bounds(terrain._octreeScale);
    if (!settings.bounds.intersects(bounds))
        return;

    LoD = terrain.get_lod()->desired_lod(*this);
    if (!_isSet)
        set_value(terrain.get_sdf()->distance(_center));

    float old_value = get_value();
    float sdf_value = settings.sdf->distance(_center - settings.position);
    float new_value = SDF::apply_operation(settings.operation, old_value, sdf_value, terrain._octreeScale);

    // set_value(settings.sdf->distance(_center - settings.position));

    if (has_surface(terrain, new_value)) // || has_surface(terrain, sdf_value)
    {
        subdivide(terrain._octreeScale);
    }
    else
    {
        if (settings.bounds.encloses(bounds))
            prune_children();
    }
    set_value(new_value);
    _isSet = true;

    if (!is_leaf())
    {
        for (auto &child : *_children)
        {
            child->modify_sdf_in_bounds(terrain, settings);
        }
    }

    if (is_chunk(terrain))
        queue_update(terrain);
    else if (_chunk != nullptr)
        delete_chunk();
}

void VoxelOctreeNode::update_chunk(JarVoxelTerrain &terrain, const ChunkMeshData *chunkMeshData)
{
    _isEnqueued = false;
    if (chunkMeshData == nullptr || !is_chunk(terrain))
    {
        delete_chunk();
        return;
    }

    if (_chunk == nullptr)
    {
        _chunk = static_cast<JarVoxelChunk *>(terrain.get_chunk_scene()->instantiate());
        terrain.add_child(_chunk);
    }

    _chunk->update_chunk(*chunkMeshData);
}

void VoxelOctreeNode::queue_update(JarVoxelTerrain &terrain)
{
    if (_isEnqueued)
        return;
    _isEnqueued = true;
    terrain.enqueue_chunk_update(*this);
}

void VoxelOctreeNode::delete_chunk()
{
    if (_chunk != nullptr)
    {
        // JarVoxelTerrain::RemoveChunk(_chunk);
        _chunk->queue_free();
    }
    _chunk = nullptr;
}

void VoxelOctreeNode::get_voxel_leaves_in_bounds(const JarVoxelTerrain &terrain, const Bounds &bounds,
                                                 std::vector<VoxelOctreeNode *> &result)
{
    if (!get_bounds(terrain._octreeScale).intersects(bounds))
        return;

    // LoD = terrain.get_lod()->desired_lod(*this);

    if (_size == LoD || (is_leaf() && _size >= LoD))
    {
        result.push_back(this);
        return;
    }

    if (is_chunk(terrain))
        for (auto &child : *_children)//use all the same LoD from here on out
            child->get_voxel_leaves_in_bounds(terrain, bounds, LoD, result);
    else
        for (auto &child : *_children)
            child->get_voxel_leaves_in_bounds(terrain, bounds, result);
}

void VoxelOctreeNode::get_voxel_leaves_in_bounds(const JarVoxelTerrain &terrain, const Bounds &bounds, const int LOD,
                                                 std::vector<VoxelOctreeNode *> &result)
{
    if (!get_bounds(terrain._octreeScale).intersects(bounds))
        return;

    if (_size == LOD || (is_leaf() && _size >= LOD))
    {
        result.push_back(this);
        return;
    }

    for (auto &child : *_children)
        child->get_voxel_leaves_in_bounds(terrain, bounds, LOD, result);
}

inline std::unique_ptr<VoxelOctreeNode> VoxelOctreeNode::create_child_node(const glm::vec3 &center, int size)
{
    return std::make_unique<VoxelOctreeNode>(this, center, size);
}
