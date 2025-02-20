#ifndef SURFACE_NETS_H
#define SURFACE_NETS_H

#include "chunk_mesh_data.h"
#include "stitched_mesh_chunk.h"
#include "mesh_compute_scheduler.h"
#include "jar_voxel_lod.h"
#include "voxel_octree_node.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <vector>

using namespace godot;

class JarVoxelTerrain;

class StitchedSurfaceNets
{
  private:
    PackedVector3Array _verts;
    PackedVector3Array _normals;
    PackedColorArray _colors;
    PackedInt32Array _indices;
    std::vector<bool> _badNormals;
    std::unordered_map<glm::ivec3, int> _edgeIndices;

    const VoxelOctreeNode *_chunk;
    StitchedMeshChunk _meshChunk;
    glm::vec3 _tempPoints[12];

    inline void add_tri(int n0, int n1, int n2, bool flip);

  public:
    StitchedSurfaceNets(const JarVoxelTerrain &terrain, const ScheduledChunk &chunk);

    ChunkMeshData *generate_mesh_data(const JarVoxelTerrain &terrain);
};

#endif // SURFACE_NETS_H
