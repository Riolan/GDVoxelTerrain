#include "surface_nets.h"
#include "fit_plane.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "jar_voxel_terrain.h"
SurfaceNets::SurfaceNets(const JarVoxelTerrain *terrain, const ScheduledChunk &chunk)
    : _chunk(chunk.node), _meshChunk(MeshChunk(terrain, chunk.node))
{
    int vertCount = _meshChunk.nodes.size();
    // _verts.reserve(vertCount);
    // _normals.reserve(vertCount);
    // _colors.reserve(vertCount);
    // _indices.reserve(vertCount * 6);
}

ChunkMeshData *SurfaceNets::generate_mesh_data(const JarVoxelTerrain *terrain)
{
    int flip = _meshChunk.Octant.x * _meshChunk.Octant.y * _meshChunk.Octant.z;
    for (size_t node_id = 0; node_id < _meshChunk.nodes.size(); node_id++)
    {
        if (_meshChunk.vertexIndices[node_id] <= -2)
            continue;
        auto neighbours = std::vector<int>();

        if (!_meshChunk.get_neighbours(_meshChunk.positions[node_id], neighbours))
            continue;

        glm::vec3 vertexPosition(0.0f);
        Color color = Color(0, 0, 0, 0);
        glm::vec3 normal(0.0f);
        int duplicates = 0, edge_crossings = 0;

        for (auto &edge : MeshChunk::Edges)
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
            (static_cast<int>(flip * glm::sign(glm::sign(_meshChunk.nodes[neighbours[6]]->get_value()) -
                                               glm::sign(_meshChunk.nodes[neighbours[7]]->get_value())) +
                              1))
                << 0 |
            (static_cast<int>(flip * glm::sign(glm::sign(_meshChunk.nodes[neighbours[7]]->get_value()) -
                                               glm::sign(_meshChunk.nodes[neighbours[5]]->get_value())) +
                              1))
                << 2 |
            (static_cast<int>(flip * glm::sign(glm::sign(_meshChunk.nodes[neighbours[3]]->get_value()) -
                                               glm::sign(_meshChunk.nodes[neighbours[7]]->get_value())) +
                              1))
                << 4;

        vertexPosition /= static_cast<float>(edge_crossings);
        color /= static_cast<float>(edge_crossings);
        normal = glm::normalize(normal);

        // if (SquareVoxels)
        // vertexPosition = node->_center();

        if (duplicates > 0)
        {
            // glm::vec3 planeNormal =
            //     FitPlane::fit(std::vector<glm::vec3>(_tempPoints, _tempPoints + edge_crossings)); //, vertexPosition);
            // if (glm::dot(planeNormal, normal) < 0)
            //     planeNormal = -planeNormal;
            // normal = planeNormal;
            normal = glm::vec3(0,1,0);
        }
        vertexPosition -= _chunk->_center;
        _meshChunk.vertexIndices[node_id] = (_verts.size());
        _verts.push_back({vertexPosition.x, vertexPosition.y, vertexPosition.z});
        _normals.push_back({normal.x, normal.y, normal.z});
        _colors.push_back(color);
    }

    if (_verts.size() == 0)
    {
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
            if (_meshChunk.get_unique_neighbouring_vertices(pos, MeshChunk::FaceOffsets[i], neighbours))
            {
                if (neighbours.size() == 4)
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
                else if (neighbours.size() == 3)
                {
                    if (_meshChunk.nodes[neighbours[0]]->_size == _meshChunk.nodes[neighbours[1]]->_size &&
                        _meshChunk.nodes[neighbours[1]]->_size == _meshChunk.nodes[neighbours[2]]->_size)
                        continue;

                    add_tri(_meshChunk.vertexIndices[neighbours[0]], _meshChunk.vertexIndices[neighbours[1]],
                            _meshChunk.vertexIndices[neighbours[2]], flipFace == -1);
                }
            }
        }
    }

    if (_indices.size() == 0)
    {
        return nullptr;
    }

    // Assign arrays to mesh data
    Array meshData;
    meshData.resize(Mesh::ARRAY_MAX);
    meshData[Mesh::ARRAY_VERTEX] = _verts;
    meshData[Mesh::ARRAY_NORMAL] = _normals;
    meshData[Mesh::ARRAY_COLOR] = _colors;
    meshData[Mesh::ARRAY_INDEX] = _indices;

    return new ChunkMeshData(meshData, _meshChunk.RealLoD, _meshChunk.IsEdgeChunk,
                             _chunk->get_bounds(terrain->_octreeScale));
}

inline void SurfaceNets::add_tri(int n0, int n1, int n2, bool flip)
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

// Create PackedVector3Array and copy data using memcpy
// PackedVector3Array packedVerts;
// packedVerts.resize(_verts.size());
// std::memcpy(packedVerts.ptrw(), _verts.data(), _verts.size() * sizeof(Vector3));

// // Create PackedVector3Array for normals
// PackedVector3Array packedNormals;
// packedNormals.resize(_normals.size());
// std::memcpy(packedNormals.ptrw(), _normals.data(), _normals.size() * sizeof(Vector3));

// // Create PackedColorArray and copy data using memcpy
// PackedColorArray packedColors;
// packedColors.resize(_colors.size());
// std::memcpy(packedColors.ptrw(), _colors.data(), _colors.size() * sizeof(Color));

// // Create PackedInt32Array and copy data using memcpy
// PackedInt32Array packedIndices;
// packedIndices.resize(_indices.size());
// std::memcpy(packedIndices.ptrw(), _indices.data(), _indices.size() * sizeof(int));