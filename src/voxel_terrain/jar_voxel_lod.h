#ifndef LEVEL_OF_DETAIL_H
#define LEVEL_OF_DETAIL_H

#include "voxel_octree_node.h"
#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>

using namespace godot;

class JarVoxelTerrain;

class JarVoxelLoD : public Resource
{
    GDCLASS(JarVoxelLoD, Resource);

  private:
    float _automaticUpdateDistance = 32;
    float _minLodDistance = 64;
    int _lodLevelCount = 8;
    bool _automaticUpdate = true;
    float _lodPadding = 0;

    int _maxChunkSize;
    float _autoMeshCoolDown;
    glm::vec3 _cameraPosition;
    std::vector<float> _lodLevels;

  protected:
    
    static void _bind_methods();

  public:
    JarVoxelLoD();

    float get_automatic_update_distance() const;
    void set_automatic_update_distance(float distance);

    float get_min_lod_distance() const;
    void set_min_lod_distance(float distance);

    int get_lod_level_count() const;
    void set_lod_level_count(int count);

    bool get_automatic_update() const;
    void set_automatic_update(bool update);

    float get_lod_padding() const;
    void set_lod_padding(float padding);

    glm::vec3 get_camera_position() const;

    void init();
    bool process(const JarVoxelTerrain &terrain, double delta);
    bool update_camera_position(const JarVoxelTerrain &terrain, const bool force);

    int desired_lod(const VoxelOctreeNode &node, float factor = 1.0f);
    int lod_at(const glm::vec3 &position) const;
    float lod_to_grid_size(const int lod) const;
    glm::vec3 snap_to_grid(const glm::vec3 pos, const float grid_size) const;
};

#endif // LEVEL_OF_DETAIL_H
