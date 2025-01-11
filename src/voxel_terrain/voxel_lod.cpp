#include "voxel_lod.h"
#include "jar_voxel_terrain.h"
#include "mesh_compute_scheduler.h"
#include <godot_cpp/variant/vector3.hpp>

VoxelLoD::VoxelLoD(bool automaticUpdate, float automaticUpdateDistance, float minLodDistance, int lodLevelCount,
                   float lodPadding, Node3D *node)
    : _node(node), _automaticUpdate(automaticUpdate), _automaticUpdateDistance(automaticUpdateDistance),
      _minLodDistance(minLodDistance), _lodLevelCount(lodLevelCount), _lodPadding(lodPadding), _autoMeshCoolDown(0.0f)
{

    //_cameraPosition = _node->get_global_transform().origin;
    _cameraPosition = {0, 0, 0};

    _lodLevels.resize(_lodLevelCount);
    _lodLevels[0] = minLodDistance;
    for (int i = 1; i < _lodLevelCount; i++)
    {
        _lodLevels[i] = _lodLevels[i - 1] * 2;
    }

    for (int i = 0; i < _lodLevelCount; i++)
    {
        _lodLevels[i] += _lodPadding;
    }

    std::transform(_lodLevels.begin(), _lodLevels.end(), _lodLevels.begin(), [](float x) { return x * x; });

    _maxChunkSize = std::max(static_cast<int>(_lodLevels.size()) - 1, 1) + 4; // 4 =
                                                                                     // JarVoxelTerrain::MinChunkSize
}

void VoxelLoD::process(double delta)
{
    _autoMeshCoolDown -= static_cast<float>(delta);
    // if (JarVoxelTerrain::IsBuilding || MeshComputeScheduler::IsMeshing) _autoMeshCoolDown = 0.1f;

    // if (_automaticUpdate && _autoMeshCoolDown < 0.0f &&
    // _cameraPosition.distance_to(_node->get_global_transform().origin) > _automaticUpdateDistance) {
    //     //update_camera_position();
    // }
}

void VoxelLoD::update_camera_position(const JarVoxelTerrain &terrain)
{
    // if (terrain::IsBuilding || MeshComputeScheduler::IsMeshing) return;
    //_cameraPosition = _node->get_global_transform().origin;
}

int VoxelLoD::desired_lod(const VoxelOctreeNode &node, float factor)
{
    auto l = node._size > _maxChunkSize ? 0 : lod_at(node._center, factor);
    // UtilityFunctions::print(l);
    return l;
}

int VoxelLoD::lod_at(const glm::vec3 &position, float factor)
{
    glm::vec3 temp = position - _cameraPosition;
    float d = dot(temp, temp) * factor;

    for (size_t i = 0; i < _lodLevels.size(); i++)
    {
        if (d < _lodLevels[i])
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}
