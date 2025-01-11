#ifndef JAR_VOXEL_CHUNK_H
#define JAR_VOXEL_CHUNK_H

#include "bounds.h"
#include "chunk_mesh_data.h"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/static_body3d.hpp>

using namespace godot;

class JarVoxelChunk : public Node3D
{
    GDCLASS(JarVoxelChunk, Node3D);

  private:
    int lod = 0;
    bool edge_chunk = false;
    Bounds bounds;

    // node references
    MeshInstance3D* mesh_instance = nullptr;
    CollisionShape3D* collision_shape = nullptr;
    StaticBody3D* static_body = nullptr;

    // data containers
    Ref<ArrayMesh> array_mesh;
    Ref<ConcavePolygonShape3D> concave_polygon_shape;
    Ref<ShaderMaterial> material;

    // std::vector<Ref<MultiMeshInstance3D>> multi_mesh_instance;
    //  Add references for grass and biome textures if necessary
    // ChunkMeshData* chunk_mesh_data;
    //  bool update_when_ready;

    static void _bind_methods();

  public:
    JarVoxelChunk();
    ~JarVoxelChunk();

    int get_lod() const;
    void set_lod(int p_lod);

    bool is_edge_chunk() const;
    void set_edge_chunk(bool p_edge_chunk);

    MeshInstance3D* get_mesh_instance() const;
    void set_mesh_instance(MeshInstance3D* p_mesh_instance);

    CollisionShape3D* get_collision_shape() const;
    void set_collision_shape(CollisionShape3D* p_collision_shape);

    StaticBody3D* get_static_body() const;
    void set_static_body(StaticBody3D* p_static_body);

    Ref<ArrayMesh> get_array_mesh() const;
    void set_array_mesh(Ref<ArrayMesh> p_array_mesh);

    Ref<ConcavePolygonShape3D> get_concave_polygon_shape() const;
    void set_concave_polygon_shape(Ref<ConcavePolygonShape3D> p_concave_polygon_shape);

    Ref<ShaderMaterial> get_material() const;
    void set_material(Ref<ShaderMaterial> p_material);

    void update_chunk(const ChunkMeshData &chunk_mesh_data);
    void update_collision_mesh();
    void delete_chunk();
};

#endif // JAR_VOXEL_CHUNK_H
