#include "stitched_mesh_chunk.h"
#include "jar_voxel_terrain.h"
#include "utils.h"

const std::vector<glm::ivec3> StitchedMeshChunk::Offsets = {
    glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0),
    glm::ivec3(0, 0, 1), glm::ivec3(1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)};

const std::vector<glm::vec3> StitchedMeshChunk::checkLodOffsets = {glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
                                                                   glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
                                                                   glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)};

const std::vector<glm::ivec2> StitchedMeshChunk::Edges = {
    glm::ivec2(0, 1), glm::ivec2(2, 3), glm::ivec2(4, 5), glm::ivec2(6, 7), glm::ivec2(0, 2), glm::ivec2(1, 3),
    glm::ivec2(4, 6), glm::ivec2(5, 7), glm::ivec2(0, 4), glm::ivec2(1, 5), glm::ivec2(2, 6), glm::ivec2(3, 7)};

const std::vector<glm::ivec3> StitchedMeshChunk::YzOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0),
                                                              glm::ivec3(0, 0, 1), glm::ivec3(0, 1, 1)};

const std::vector<glm::ivec3> StitchedMeshChunk::XzOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0),
                                                              glm::ivec3(0, 0, 1), glm::ivec3(1, 0, 1)};

const std::vector<glm::ivec3> StitchedMeshChunk::XyOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0),
                                                              glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0)};

const std::vector<std::vector<glm::ivec3>> StitchedMeshChunk::FaceOffsets = {YzOffsets, XzOffsets, XyOffsets};

StitchedMeshChunk::StitchedMeshChunk(const JarVoxelTerrain &terrain, const VoxelOctreeNode &chunk)
{
    glm::vec3 chunkCenter = chunk._center;
    auto cameraPosition = terrain.get_lod()->get_camera_position();
    float leafSize = ((1 << chunk.LoD) * terrain.get_octree_scale());
    Bounds bounds = chunk.get_bounds(terrain._octreeScale).expanded(leafSize - 0.001f);
    nodes.clear();
    terrain._voxelRoot->get_voxel_leaves_in_bounds(terrain, bounds, chunk.LoD, nodes);
    bounds = bounds.expanded(0.001f);

    if (nodes.empty())
        return;

    // find if there are any lod boundaries
    const float edge_length = chunk.edge_length(terrain.get_octree_scale());
    _lodL2HBoundaries = _lodH2LBoundaries = 0;
    for (size_t i = 0; i < checkLodOffsets.size(); i++)
    {
        int lod = terrain.get_lod()->lod_at(chunk._center + edge_length * checkLodOffsets[i]);
        _lodL2HBoundaries |= (chunk.LoD > lod ? 1 : 0) << i;
        _lodH2LBoundaries |= (chunk.LoD < lod ? 1 : 0) << i;
    }

    float maxChunkSize = (1 << chunk.LoD + 1);
    float normalizingFactor = 1.0f / leafSize;
    half_leaf_size = glm::vec3(leafSize * 0.5);
    glm::vec3 minPos = bounds.min;
    glm::ivec3 clampMax = glm::ivec3(LargestPos);

    positions.clear();
    vertexIndices.clear();
    faceDirs.clear();
    _leavesLut.clear();
    positions.resize(nodes.size(), glm::ivec3(0));
    vertexIndices.resize(nodes.size(), -2);
    faceDirs.resize(nodes.size(), 0);
    _leavesLut.resize(ChunkRes * ChunkRes * ChunkRes, 0);

    for (size_t i = 0; i < nodes.size(); i++)
    {
        VoxelOctreeNode *node = nodes[i];
        Bounds b = node->get_bounds(terrain._octreeScale);

        if (node->LoD < 0 || node->_size > maxChunkSize || !b.intersects(bounds))
            continue;

        auto overlap = bounds.intersected(b);
        glm::vec3 min = (overlap.min - minPos) * normalizingFactor;
        glm::vec3 max = (overlap.max - minPos) * normalizingFactor;
        glm::ivec3 intMin = glm::clamp((glm::ivec3)glm::floor(min), glm::ivec3(0.0f), clampMax);
        glm::ivec3 intMax = glm::clamp((glm::ivec3)glm::ceil(max) - glm::ivec3(1.0f), glm::ivec3(0.0f), clampMax);

        glm::ivec3 pos = intMax;
        positions[i] = pos;

        bool isSmall = intMax == intMin;
        if (!isSmall || is_on_boundary(_lodH2LBoundaries, pos))
            continue;

        vertexIndices[i] = -1;

        if (isSmall)
        {
            _leavesLut[intMax.x + ChunkRes * (intMax.y + ChunkRes * intMax.z)] = i + 1;
        }
    }
}

bool StitchedMeshChunk::should_have_quad(const glm::ivec3 &position, const int face) const
{
    //we might also need some cases for l2h chunks i think
    switch (face)
    {
    case 0:
        return position.x < LargestPos;
    case 1:
        return position.y < LargestPos;
    case 2:
        return position.z < LargestPos;
    default:
        return true;
    }
}

inline int StitchedMeshChunk::get_node_index_at(const glm::ivec3 &pos) const
{
    if (pos.x < 0 || pos.x >= ChunkRes || pos.y < 0 || pos.y >= ChunkRes || pos.z < 0 || pos.z >= ChunkRes)
        return -1;
    else
        return (_leavesLut[pos.x + ChunkRes * (pos.y + ChunkRes * pos.z)] - 1);
}

bool StitchedMeshChunk::get_unique_neighbouring_vertices(const glm::ivec3 &pos, const std::vector<glm::ivec3> &offsets,
                                                         std::vector<int> &result) const
{
    for (const auto &o : offsets)
    {
        auto n = get_node_index_at(pos + o);
        if (n < 0 || vertexIndices[n] < 0)
        {
            return false;
        }
        if (std::find(result.begin(), result.end(), n) == result.end())
            result.push_back(n);
    }
    return true;
}

bool StitchedMeshChunk::get_neighbours(const glm::ivec3 &pos, std::vector<int> &result) const
{
    for (const auto &o : StitchedMeshChunk::Offsets)
    {
        auto n = get_node_index_at(pos + o);
        if (n < 0)
        {
            return false;
        }
        result.push_back(n);
    }
    return true;
}
