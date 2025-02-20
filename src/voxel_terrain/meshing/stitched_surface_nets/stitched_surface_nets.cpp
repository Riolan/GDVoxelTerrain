#include "stitched_surface_nets.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "jar_voxel_terrain.h"
StitchedSurfaceNets::StitchedSurfaceNets(const JarVoxelTerrain &terrain, const ScheduledChunk &chunk)
    : _chunk(&chunk.node), _meshChunk(StitchedMeshChunk(terrain, chunk.node))
{
    int vertCount = _meshChunk.nodes.size();
}

ChunkMeshData *StitchedSurfaceNets::generate_mesh_data(const JarVoxelTerrain &terrain)
{
    // if(_meshChunk.is_edge_chunk())
    //      return nullptr;
    //shell stitching idea:
    // get all vertices in a ring around the chunk based on aabb/lod thingy
    // we will assume that these are homogenous in size 2x that of the primary ones
    // process them in the same way as the primary ones to get vertices
    // then, process main chunk as we do right now
    // then, process the ring vertices by iterating through the ones with vertices
    // most importantly, we need two things: 1 way to easily interface between both ring and main chunk, and 2: the direction to check in.
    //  1: assuming homogenous and 2x size of ring, we divide by 2 to get the main chunk position.
    // 2: can be solved using octant relative to chunk center? 

    for (size_t node_id = 0; node_id < _meshChunk.nodes.size(); node_id++)
    {
        if (_meshChunk.vertexIndices[node_id] <= -2)
            continue;
        auto neighbours = std::vector<int>();
        glm::ivec3 grid_position = _meshChunk.positions[node_id];

        if (!_meshChunk.get_neighbours(grid_position, neighbours))
            continue;

        glm::vec3 vertexPosition(0.0f);
        Color color = Color(0, 0, 0, 0);
        glm::vec3 normal(0.0f);
        int duplicates = 0, edge_crossings = 0;
        for (auto &edge : StitchedMeshChunk::Edges)
        {
            auto ai = neighbours[edge.x];
            auto bi = neighbours[edge.y];
            if (ai == bi)
            {
                duplicates++;
                continue;
            }
            auto na = _meshChunk.nodes[ai];
            auto nb = _meshChunk.nodes[bi];

            float valueA = na->get_value();
            float valueB = nb->get_value();
            glm::vec3 posA = na->_center;
            glm::vec3 posB = nb->_center;

            normal += (valueB - valueA) * (posB - posA);

            if (glm::sign(valueA) == glm::sign(valueB))
                continue;

            // Color colorA = na->get_node_color();
            // Color colorB = nb->get_node_color();

            float t = glm::abs(valueA) / (glm::abs(valueA) + glm::abs(valueB));
            glm::vec3 point = glm::mix(posA, posB, t);
            vertexPosition += point;
            _tempPoints[edge_crossings++] = point;
            // color += color.linear_interpolate(colorA.linear_interpolate(colorB, t), 1.0f);
        }

        if (edge_crossings <= 0)
        {
            // UtilityFunctions::print("No crossings!");
            continue;
        }

        _meshChunk.faceDirs[node_id] =
            (static_cast<int>(glm::sign(glm::sign(_meshChunk.nodes[neighbours[6]]->get_value()) -
                                        glm::sign(_meshChunk.nodes[neighbours[7]]->get_value())) +
                              1))
                << 0 |
            (static_cast<int>(glm::sign(glm::sign(_meshChunk.nodes[neighbours[7]]->get_value()) -
                                        glm::sign(_meshChunk.nodes[neighbours[5]]->get_value())) +
                              1))
                << 2 |
            (static_cast<int>(glm::sign(glm::sign(_meshChunk.nodes[neighbours[3]]->get_value()) -
                                        glm::sign(_meshChunk.nodes[neighbours[7]]->get_value())) +
                              1))
                << 4;

        vertexPosition /= static_cast<float>(edge_crossings);
        color /= static_cast<float>(edge_crossings);
        normal = glm::normalize(normal);

        // if (SquareVoxels)
        // vertexPosition = _meshChunk.nodes[node_id]->_center;

        vertexPosition -= _chunk->_center;
        int vertexIndex = (_verts.size());

        if (_meshChunk.is_on_any_boundary(grid_position))
            _edgeIndices[grid_position] = (vertexIndex);

        _meshChunk.vertexIndices[node_id] = vertexIndex;
        _verts.push_back({vertexPosition.x, vertexPosition.y, vertexPosition.z});
        _normals.push_back({normal.x, normal.y, normal.z});
        _colors.push_back(color);
    }

    if (_verts.size() == 0)
    {
        // UtilityFunctions::printerr("No vertices!");
        return nullptr;
    }

    for (size_t node_id = 0; node_id < _meshChunk.nodes.size(); node_id++)
    {
        if (_meshChunk.vertexIndices[node_id] <= -1)
            continue;

        const int faces = 3;
        auto pos = _meshChunk.positions[node_id];
        auto faceDirs = _meshChunk.faceDirs[node_id];
        for (int i = 0; i < faces; i++)
        {
            int flipFace = ((faceDirs >> (2 * i)) & 3) - 1;
            if (flipFace == 0 || !_meshChunk.should_have_quad(pos, i))
                continue;

            auto neighbours = std::vector<int>();
            if (_meshChunk.get_unique_neighbouring_vertices(pos, StitchedMeshChunk::FaceOffsets[i], neighbours) &&
                neighbours.size() == 4)
            {

                int n0 = _meshChunk.vertexIndices[neighbours[0]];
                int n1 = _meshChunk.vertexIndices[neighbours[1]];
                int n2 = _meshChunk.vertexIndices[neighbours[2]];
                int n3 = _meshChunk.vertexIndices[neighbours[3]];
                if (_verts[n0].distance_squared_to(_verts[n3]) < _verts[n1].distance_squared_to(_verts[n2]))
                {
                    add_tri(n0, n1, n3, flipFace == -1);
                    add_tri(n0, n3, n2, flipFace == -1);
                }
                else
                {
                    add_tri(n1, n3, n2, flipFace == -1);
                    add_tri(n1, n2, n0, flipFace == -1);
                }
            }
        }
    }

    if (_indices.size() == 0)
    {
        // UtilityFunctions::printerr("No indices!");
        return nullptr;
    }

    // for (size_t vert_id = 0; vert_id < _normals.size(); vert_id++)
    // {
    //     _normals[vert_id] = _normals[vert_id].normalized();
    // }

    // Assign arrays to mesh data
    Array meshData;
    meshData.resize(Mesh::ARRAY_MAX);
    meshData[Mesh::ARRAY_VERTEX] = _verts;
    meshData[Mesh::ARRAY_NORMAL] = _normals;
    meshData[Mesh::ARRAY_COLOR] = _colors;
    meshData[Mesh::ARRAY_INDEX] = _indices;

    ChunkMeshData *output =
        new ChunkMeshData(meshData, _chunk->LoD, _meshChunk.is_edge_chunk(), _chunk->get_bounds(terrain._octreeScale));
    output->edgeIndices = _edgeIndices;
    return output;
}

inline void StitchedSurfaceNets::add_tri(int n0, int n1, int n2, bool flip)
{
    if (!flip)
    {
        _indices.push_back(n0);
        _indices.push_back(n1);
        _indices.push_back(n2);
    }
    else
    {
        _indices.push_back(n1);
        _indices.push_back(n0);
        _indices.push_back(n2);
    }
}