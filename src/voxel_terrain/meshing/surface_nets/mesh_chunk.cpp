#include "mesh_chunk.h"
#include "jar_voxel_terrain.h"
#include "utils.h"

const std::vector<glm::ivec3> MeshChunk::Offsets = {glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0),
                                                    glm::ivec3(1, 1, 0), glm::ivec3(0, 0, 1), glm::ivec3(1, 0, 1),
                                                    glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)};

const std::vector<glm::ivec2> MeshChunk::Edges = {
    glm::ivec2(0, 1), glm::ivec2(2, 3), glm::ivec2(4, 5), glm::ivec2(6, 7), glm::ivec2(0, 2), glm::ivec2(1, 3),
    glm::ivec2(4, 6), glm::ivec2(5, 7), glm::ivec2(0, 4), glm::ivec2(1, 5), glm::ivec2(2, 6), glm::ivec2(3, 7)};

const std::vector<glm::ivec3> MeshChunk::YzOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1),
                                                      glm::ivec3(0, 1, 1)};

const std::vector<glm::ivec3> MeshChunk::XzOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 0, 1),
                                                      glm::ivec3(1, 0, 1)};

const std::vector<glm::ivec3> MeshChunk::XyOffsets = {glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0),
                                                      glm::ivec3(1, 1, 0)};

const std::vector<std::vector<glm::ivec3>> MeshChunk::FaceOffsets = {YzOffsets, XzOffsets, XyOffsets};

MeshChunk::MeshChunk(const JarVoxelTerrain &terrain, const VoxelOctreeNode &chunk)
{
    glm::vec3 chunkCenter = chunk._center;
    auto cameraPosition = terrain.get_lod()->get_camera_position();
    Octant = glm::ivec3(chunkCenter.x > cameraPosition.x ? 1 : -1, chunkCenter.y > cameraPosition.y ? 1 : -1,
                        chunkCenter.z > cameraPosition.z ? 1 : -1);

    float leafSize = ((1 << chunk.LoD) * terrain.get_octree_scale());
    Bounds bounds = chunk.get_bounds(terrain._octreeScale).expanded(leafSize - 0.001f);

    //UtilityFunctions::print("Bounds: " + Utils::to_string(bounds));

    nodes.clear();
    terrain.get_voxel_leaves_in_bounds(bounds, nodes);
    bounds = bounds.expanded(0.001f);

    if (nodes.empty())
        return;

    int chunkLoD = chunk.LoD;
    RealLoD = chunk.LoD;
    _chunkResolution = ChunkRes;

    for (const VoxelOctreeNode *n : nodes)
    {
        int l = n->LoD;
        if (l > chunkLoD)
            IsEdgeChunk = true;
        if (l >= chunkLoD)
            continue;

        chunkLoD = chunk.LoD - 1;
        leafSize = ((1 << chunkLoD) * terrain.get_octree_scale());
        _chunkResolution = LargeChunkRes;
        IsEdgeChunk = true;
        break;
    }

    // UtilityFunctions::print("NodeCount: " + godot::String(std::to_string(nodes.size()).c_str()));

    float maxChunkSize = (1 << chunkLoD + 1);
    float normalizingFactor = 1.0f / leafSize;
    half_leaf_size = glm::vec3(leafSize * 0.5);
    glm::vec3 minPos = bounds.min;
    glm::ivec3 clampMax = glm::ivec3(_chunkResolution - 1);

    positions.clear();
    vertexIndices.clear();
    faceDirs.clear();
    _leavesLut.clear();
    positions.resize(nodes.size(), glm::ivec3(0));
    vertexIndices.resize(nodes.size(), -2);
    faceDirs.resize(nodes.size(), 0);
    _leavesLut.resize(_chunkResolution * _chunkResolution * _chunkResolution, 0);

    for (size_t i = 0; i < nodes.size(); i++)
    {
        VoxelOctreeNode *node = nodes[i];
        Bounds b = node->get_bounds(terrain._octreeScale);

        if (node->LoD < 0 || node->_size > maxChunkSize || !b.intersects(bounds))
            continue;

        auto overlap = bounds.intersected(b);
        glm::vec3 min = (overlap.min - minPos) * normalizingFactor;
        glm::vec3 max = (overlap.max - minPos) * normalizingFactor;
        glm::ivec3 intMin = glm::clamp((glm::ivec3)glm::floor(min), glm::ivec3(0.0f), glm::ivec3(clampMax));
        glm::ivec3 intMax = glm::clamp((glm::ivec3)glm::ceil(max) - glm::ivec3(1.0f), glm::ivec3(0.0f), glm::ivec3(clampMax));

        positions[i] = (glm::ivec3(Octant.x == -1 ? intMin.x : intMax.x, Octant.y == -1 ? intMin.y : intMax.y,
                                   Octant.z == -1 ? intMin.z : intMax.z));
        vertexIndices[i] = -1;

        if (intMax == intMin)
        {
            _leavesLut[intMax.x + _chunkResolution * (intMax.y + _chunkResolution * intMax.z)] = i + 1;
        }
        else
        {
            for (int x = intMin.x; x <= intMax.x; x++)
            {
                for (int y = intMin.y; y <= intMax.y; y++)
                {
                    for (int z = intMin.z; z <= intMax.z; z++)
                    {
                        _leavesLut[x + _chunkResolution * (y + _chunkResolution * z)] = i + 1;
                    }
                }
            }
        }
    }
}

bool MeshChunk::should_have_quad(const glm::ivec3 &position, const int face) const
{
    switch (face)
    {
    case 0:
        return position.x > 0;// && position.x < _chunkResolution - 1;
    case 1:
        return position.y > 0;// && position.y < _chunkResolution - 1;
    case 2:
        return position.z > 0;// && position.z < _chunkResolution - 1;
    default:
        return true;
    }
}

inline int MeshChunk::get_node_index_at(const glm::ivec3 &pos) const
{
    if (pos.x < 0 || pos.x >= _chunkResolution || pos.y < 0 || pos.y >= _chunkResolution || pos.z < 0 ||
        pos.z >= _chunkResolution)
        return -1;
    else
        return (_leavesLut[pos.x + _chunkResolution * (pos.y + _chunkResolution * pos.z)] - 1);
}

bool MeshChunk::get_unique_neighbouring_vertices(const glm::ivec3 &pos,
                                                             const std::vector<glm::ivec3> &offsets, 
                                                             std::vector<int> &result) const
{
    for (const auto &o : offsets)
    {
        auto n = get_node_index_at(pos + o * Octant);
        if (n < 0 || vertexIndices[n] < 0)
        {
            return false;
        }
        if(std::find(result.begin(), result.end(), n) == result.end())
            result.push_back(n);
    }
    return true;
}

bool MeshChunk::get_neighbours(const glm::ivec3 &pos, std::vector<int> &result) const
{
    for (const auto &o : MeshChunk::Offsets)
    {
        auto n = get_node_index_at(pos + o * Octant);
        if (n < 0)
        {
            return false;
        }
        result.push_back(n);
    }
    return true;
}
