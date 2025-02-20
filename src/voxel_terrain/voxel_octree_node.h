#ifndef VOXEL_OCTREE_NODE_H
#define VOXEL_OCTREE_NODE_H

#include "octree_node.h"
#include "jar_voxel_chunk.h"
#include "modify_settings.h"
#include "bounds.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <memory>
#include <algorithm>

class JarVoxelTerrain;

class VoxelOctreeNode : public OctreeNode<VoxelOctreeNode> {
public:
    float _value = 0;
    glm::vec4 NodeColor{0, 0, 0, 0};

    bool _isSet = false;
    bool _isDirty = false;
    bool _isEnqueued = false;

    JarVoxelChunk* _chunk = nullptr;

    int LoD = 0;

    bool is_dirty() const;
    void set_dirty(bool value);
    float get_value();
    void set_value(float value);

    inline bool is_chunk(const JarVoxelTerrain &terrain) const;
    inline bool is_one_above_chunk(const JarVoxelTerrain &terrain) const;
    inline bool is_not_edge_chunk(const JarVoxelTerrain &terrain) const;

    void populateUniqueLoDValues(std::vector<int>& lodValues) const;

    bool is_enqueued() const;
    bool is_parent_enqueued() const;    
    bool is_any_children_enqueued() const;

    inline bool should_delete_chunk(const JarVoxelTerrain &terrain) const;
public:
    VoxelOctreeNode(int size);
    VoxelOctreeNode(VoxelOctreeNode* parent, const glm::vec3 center, int size);

    int priority() const;

    void build(JarVoxelTerrain &terrain);
    
    inline bool has_surface(const JarVoxelTerrain &terrain, const float value);
    void queue_update(JarVoxelTerrain &terrain);
    void modify_sdf_in_bounds(JarVoxelTerrain &terrain, const ModifySettings& settings);
    void update_chunk(JarVoxelTerrain &terrain, const ChunkMeshData* chunkMeshData);
    
    void delete_chunk();
    void get_voxel_leaves_in_bounds(const JarVoxelTerrain &terrain, const Bounds& Bounds, std::vector<VoxelOctreeNode*>& result);

private:
    void get_voxel_leaves_in_bounds(const JarVoxelTerrain &terrain, const Bounds& Bounds, const int LOD, std::vector<VoxelOctreeNode*>& result);


protected:
    inline virtual std::unique_ptr<VoxelOctreeNode> create_child_node(const glm::vec3& center, int size) override;
};

#endif // VOXEL_OCTREE_NODE_H
