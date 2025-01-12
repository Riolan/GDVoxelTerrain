#ifndef SURFACE_NETS_H
#define SURFACE_NETS_H

#include "chunk_mesh_data.h"
#include "mesh_chunk.h"
#include "mesh_compute_scheduler.h"
#include "jar_voxel_lod.h"
#include "voxel_octree_node.h"
#include <glm/glm.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <vector>

using namespace godot;

class JarVoxelTerrain;

class SurfaceNets
{
  private:
    static const bool SquareVoxels = false;

    // std::vector<glm::vec3> _verts;
    // std::vector<glm::vec3> _normals;
    // std::vector<Color> _colors;
    // std::vector<int> _indices;
    PackedVector3Array _verts;
    PackedVector3Array _normals;
    PackedColorArray _colors;
    PackedInt32Array _indices;

    const VoxelOctreeNode *_chunk;
    MeshChunk _meshChunk;
    glm::vec3 _tempPoints[12];

    inline void add_tri(int n0, int n1, int n2, bool flip);

  public:
    SurfaceNets(const JarVoxelTerrain *terrain, const ScheduledChunk &chunk);

    ChunkMeshData *generate_mesh_data(const JarVoxelTerrain *terrain);
};

#endif // SURFACE_NETS_H
