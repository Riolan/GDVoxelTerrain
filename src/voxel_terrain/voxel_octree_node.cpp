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
        _value = _parent->_value;
        _isModified = _parent->_isModified;
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
    set_dirty(false);
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

inline bool VoxelOctreeNode::is_chunk(const JarVoxelTerrain *terrain) const
{
    return _size == (LoD + terrain->get_min_chunk_size());
}

inline bool VoxelOctreeNode::is_one_above_chunk(const JarVoxelTerrain *terrain) const
{
    return _size == (LoD + terrain->get_min_chunk_size() + 1);
}

inline bool VoxelOctreeNode::is_not_edge_chunk(const JarVoxelTerrain *terrain) const
{
    return true;
    // return LoD == terrain->get_lod()->lod_at(_center + glm::vec3(edge_length(), 0, 0)) &&
    //        LoD == terrain->get_lod()->lod_at(_center - glm::vec3(edge_length(), 0, 0)) &&
    //        LoD == terrain->get_lod()->lod_at(_center + glm::vec3(0, edge_length(), 0)) &&
    //        LoD == terrain->get_lod()->lod_at(_center - glm::vec3(0, edge_length(), 0)) &&
    //        LoD == terrain->get_lod()->lod_at(_center + glm::vec3(0, 0, edge_length())) &&
    //        LoD == terrain->get_lod()->lod_at(_center - glm::vec3(0, 0, edge_length()));
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

void VoxelOctreeNode::build(JarVoxelTerrain *terrain, bool ignoreLoD)
{
    build(terrain, is_chunk(terrain) || is_one_above_chunk(terrain) ? LoD : std::numeric_limits<int>::min(), ignoreLoD);
}

void VoxelOctreeNode::build(JarVoxelTerrain *terrain, int chunkLoD, bool ignoreLoD)
{
    if (!ignoreLoD)
    {
        LoD = (chunkLoD == std::numeric_limits<int>::min()) ? terrain->get_lod()->desired_lod(*this) : chunkLoD;
    }

    if (!is_chunk(terrain))
    {
        delete_chunk();
    }

    if (LoD < 0 && !ignoreLoD)
        return;

    if (!_isSet && !_isModified)
    {
        bool shouldSubdivide = set_terrain_sdf(terrain);
        if (shouldSubdivide && (_size > LoD || ignoreLoD))
        {
            subdivide(terrain->_octreeScale);
            _isSet = true;
        }
        if (_size == min_size())
            _isSet = true;
    }

    if (!is_leaf() && !(is_chunk(terrain) && _chunk != nullptr))
    {
        if (is_chunk(terrain) || is_one_above_chunk(terrain))
            chunkLoD = LoD;

        for (auto &child : *_children)
        {
            child->build(terrain, chunkLoD, ignoreLoD);
        }
    }

    if (is_chunk(terrain) && !is_leaf() && (_chunk == nullptr))
    { // && (_chunk == nullptr || _chunk->get_lod() != LoD || _chunk->is_edge_chunk() ||
      // !is_not_edge_chunk(terrain)) && !is_leaf()) {
        queue_update(terrain);
    }
}

bool VoxelOctreeNode::has_surface(const JarVoxelTerrain *terrain, float value)
{
    return std::abs(value) <
           (1 << _size) * terrain->_octreeScale * 1.44224957f; //(3*(1/2)^3)^(1/3) = 1.44224957 for d instead of r
}

bool VoxelOctreeNode::set_terrain_sdf(const JarVoxelTerrain *terrain)
{
    float value = terrain->get_sdf()->distance(_center);
    set_value(value);
    return has_surface(terrain, value);
}

void VoxelOctreeNode::modify(float newValue)
{
    float v = get_value();
    set_value(newValue);
    _isSet = true;
    _isModified = true;
    // if (std::abs(v - newValue) > 0.1f)
    // {
    //     NodeColor.r = 1.0f;
    // }
}

void VoxelOctreeNode::modify_sdf_in_bounds(JarVoxelTerrain *terrain, const ModifySettings &settings)
{
    if (settings.sdf.is_null())
    {
        UtilityFunctions::print("sdf invalid");
        return;
    }

    auto bounds = get_bounds(terrain->_octreeScale);
    if (!settings.bounds.intersects(bounds))
        return;

    float newValue = SDF::apply_operation(settings.operation, get_value(),
                                          settings.sdf->distance(_center - settings.position), terrain->_octreeScale);
    if (!_isSet)
        build(terrain, true);    

    if (!is_leaf() || _size > 0 && has_surface(terrain, newValue))
    {
        if (is_leaf())
            subdivide(terrain->_octreeScale);
        _isSet = true;
        _isModified = true;
        for (auto &child : *_children)
        {
            child->modify_sdf_in_bounds(terrain, settings);
        }
    }
    else if (settings.bounds.encloses(bounds))
    {
        modify(newValue);
        prune_children();
        delete_chunk();
    }

    if (is_chunk(terrain))
        queue_update(terrain);
    else if (_chunk != nullptr)
        delete_chunk();
}

void VoxelOctreeNode::update_chunk(JarVoxelTerrain *terrain, const ChunkMeshData *chunkMeshData)
{
    _isEnqueued = false;
    if (chunkMeshData == nullptr || LoD < 0 || is_leaf())
    {
        delete_chunk();
        return;
    }

    if (_chunk == nullptr)
    {
        _chunk = static_cast<JarVoxelChunk *>(terrain->get_chunk_scene()->instantiate());
        terrain->add_child(_chunk);
    }

    _chunk->update_chunk(*chunkMeshData);
}

void VoxelOctreeNode::queue_update(JarVoxelTerrain *terrain)
{
    if (_isEnqueued)
        return;
    _isEnqueued = true;
    terrain->get_mesh_scheduler()->enqueue(this);
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

void VoxelOctreeNode::get_voxel_leaves_in_bounds(const JarVoxelTerrain *terrain, const Bounds &bounds,
                                                 std::vector<VoxelOctreeNode *> &result)
{
    if (!get_bounds(terrain->_octreeScale).intersects(bounds))
        return;

    // int minSize = 1 << LoD;
    if (_size == LoD || (is_leaf() && _size >= LoD))
    {
        result.push_back(this);
        return;
    }

    if (is_leaf())
        return;

    for (auto &child : *_children)
    {
        child->get_voxel_leaves_in_bounds(terrain, bounds, result);
    }
}

inline VoxelOctreeNode *VoxelOctreeNode::create_child_node(const glm::vec3 &center, int size)
{
    return new VoxelOctreeNode(this, center, size);
}
