#ifndef LEVEL_OF_DETAIL_H
#define LEVEL_OF_DETAIL_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <vector>
#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include "voxel_octree_node.h"

using namespace godot;

class JarVoxelTerrain;

class VoxelLoD {
private:
    Node3D* _node;
    glm::vec3 _cameraPosition;

    std::vector<float> _lodLevels;
    float _automaticUpdateDistance;
    float _minLodDistance;
    int _lodLevelCount;
    int _maxChunkSize;
    bool _automaticUpdate;
    float _autoMeshCoolDown;
    float _lodPadding;

public:
    VoxelLoD(bool automaticUpdate, float automaticUpdateDistance, float minLodDistance, int lodLevelCount, float lodPadding, Node3D* node);

    glm::vec3 get_camera_position() const { return _cameraPosition; }
    void process(double delta);

    void update_camera_position(const JarVoxelTerrain& terrain);
    
    int desired_lod(const VoxelOctreeNode& node, float factor = 1.0f);

    int lod_at(const glm::vec3& position, float factor = 1.0f);
};

#endif // LEVEL_OF_DETAIL_H
