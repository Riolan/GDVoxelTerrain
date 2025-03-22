#include "jar_voxel_chunk.h"
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

void JarVoxelChunk::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_mesh_instance"), &JarVoxelChunk::get_mesh_instance);
    ClassDB::bind_method(D_METHOD("set_mesh_instance", "mesh_instance"), &JarVoxelChunk::set_mesh_instance);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_instance", PROPERTY_HINT_NODE_TYPE, "MeshInstance3D"),
                 "set_mesh_instance", "get_mesh_instance");

    ClassDB::bind_method(D_METHOD("get_collision_shape"), &JarVoxelChunk::get_collision_shape);
    ClassDB::bind_method(D_METHOD("set_collision_shape", "collision_shape"), &JarVoxelChunk::set_collision_shape);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "collision_shape", PROPERTY_HINT_NODE_TYPE, "CollisionShape3D"),
                 "set_collision_shape", "get_collision_shape");

    ClassDB::bind_method(D_METHOD("get_static_body"), &JarVoxelChunk::get_static_body);
    ClassDB::bind_method(D_METHOD("set_static_body", "static_body"), &JarVoxelChunk::set_static_body);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "static_body", PROPERTY_HINT_NODE_TYPE, "StaticBody3D"),
                 "set_static_body", "get_static_body");

    // ClassDB::bind_method(D_METHOD("get_array_mesh"), &JarVoxelChunk::get_array_mesh);
    // ClassDB::bind_method(D_METHOD("set_array_mesh", "array_mesh"), &JarVoxelChunk::set_array_mesh);
    // ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "array_mesh", PROPERTY_HINT_RESOURCE_TYPE, "ArrayMesh"),
    //              "set_array_mesh", "get_array_mesh");

    // ClassDB::bind_method(D_METHOD("get_concave_polygon_shape"), &JarVoxelChunk::get_concave_polygon_shape);
    // ClassDB::bind_method(D_METHOD("set_concave_polygon_shape", "concave_polygon_shape"),
    //                      &JarVoxelChunk::set_concave_polygon_shape);
    // ADD_PROPERTY(
    //     PropertyInfo(Variant::OBJECT, "concave_polygon_shape", PROPERTY_HINT_RESOURCE_TYPE, "ConcavePolygonShape3D"),
    //     "set_concave_polygon_shape", "get_concave_polygon_shape");

    // ClassDB::bind_method(D_METHOD("get_material"), &JarVoxelChunk::get_material);
    // ClassDB::bind_method(D_METHOD("set_material", "material"), &JarVoxelChunk::set_material);
    // ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
    //              "set_material", "get_material");
}

JarVoxelChunk::JarVoxelChunk() : lod(0), edge_chunk(false)
{
}

JarVoxelChunk::~JarVoxelChunk()
{
}

int JarVoxelChunk::get_lod() const
{
    return lod;
}

void JarVoxelChunk::set_lod(int p_lod)
{
    lod = p_lod;
}

uint8_t JarVoxelChunk::get_h2l_boundaries() const
{
    return h2l_boundaries;
}

void JarVoxelChunk::set_h2l_boundaries(uint8_t p_h2lboundaries)
{
    h2l_boundaries = p_h2lboundaries;
}

bool JarVoxelChunk::is_edge_chunk() const
{
    return edge_chunk;
}

void JarVoxelChunk::set_edge_chunk(bool p_edge_chunk)
{
    edge_chunk = p_edge_chunk;
}

MeshInstance3D *JarVoxelChunk::get_mesh_instance() const
{
    return mesh_instance;
}

void JarVoxelChunk::set_mesh_instance(MeshInstance3D *p_mesh_instance)
{
    mesh_instance = p_mesh_instance;
}

CollisionShape3D *JarVoxelChunk::get_collision_shape() const
{
    return collision_shape;
}

void JarVoxelChunk::set_collision_shape(CollisionShape3D *p_collision_shape)
{
    collision_shape = p_collision_shape;
}

StaticBody3D *JarVoxelChunk::get_static_body() const
{
    return static_body;
}

void JarVoxelChunk::set_static_body(StaticBody3D *p_static_body)
{
    static_body = p_static_body;
}

Ref<ArrayMesh> JarVoxelChunk::get_array_mesh() const
{
    return array_mesh;
}

void JarVoxelChunk::set_array_mesh(Ref<ArrayMesh> p_array_mesh)
{
    array_mesh = p_array_mesh;
}

Ref<ConcavePolygonShape3D> JarVoxelChunk::get_concave_polygon_shape() const
{
    return concave_polygon_shape;
}

void JarVoxelChunk::set_concave_polygon_shape(Ref<ConcavePolygonShape3D> p_concave_polygon_shape)
{
    concave_polygon_shape = p_concave_polygon_shape;
}

Ref<ShaderMaterial> JarVoxelChunk::get_material() const
{
    return material;
}

void JarVoxelChunk::set_material(Ref<ShaderMaterial> p_material)
{
    material = p_material;
}

// void JarVoxelChunk::_ready()
// {
//     array_mesh = Ref<ArrayMesh>(Object::cast_to<ArrayMesh>(*mesh_instance->get_mesh()));
//     concave_polygon_shape =
//     Ref<ConcavePolygonShape3D>(Object::cast_to<ConcavePolygonShape3D>(*collision_shape->get_shape())); material =
//     Ref<ShaderMaterial>(Object::cast_to<ShaderMaterial>(*mesh_instance->get_material_override()));
// }

void JarVoxelChunk::update_chunk(const ChunkMeshData &chunk_mesh_data)
{
    array_mesh = Ref<ArrayMesh>(Object::cast_to<ArrayMesh>(*mesh_instance->get_mesh()));
    concave_polygon_shape = Ref<ConcavePolygonShape3D>(Object::cast_to<ConcavePolygonShape3D>(*collision_shape->get_shape()));
    material = Ref<ShaderMaterial>(Object::cast_to<ShaderMaterial>(*mesh_instance->get_material_override()));
    lod = chunk_mesh_data.lod;
    h2l_boundaries = chunk_mesh_data.h2l_boundaries;
    edge_chunk = chunk_mesh_data.edge_chunk;
    auto old_bounds = bounds;
    bounds = chunk_mesh_data.bounds;
    auto position = bounds.get_center();
    // auto position = bounds.get_center() * 1.05f;
    set_position({position.x, position.y, position.z});

    array_mesh->clear_surfaces();
    array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, chunk_mesh_data.mesh_array);

    if (lod > 1)
    {
        collision_shape->set_disabled(true);
    }
    else
    {
        // collision_shape->set_disabled(!chunk_mesh_data->has_collision_mesh());
        concave_polygon_shape->set_faces(chunk_mesh_data.create_collision_mesh());
    }

    // Ref<StandardMaterial3D> stitch_material;
    // stitch_material.instantiate();
    // stitch_material->set_albedo(Color(1, 0, 1));
    // Ref<SphereMesh> sphere_mesh;
    // sphere_mesh.instantiate();
    // sphere_mesh->set_radius(0.4f);
    // sphere_mesh->set_height(0.8f);
    // PackedVector3Array verts = chunk_mesh_data.mesh_array[Mesh::ARRAY_VERTEX];
    // display all vertices: 
    // for (auto& [position, vertexId]: chunk_mesh_data.edgeVertices) 
    // {  
    //     Vector3 nodeCenter = verts[vertexId];
    //     MeshInstance3D *sphereInstance = memnew(MeshInstance3D);
    //     add_child(sphereInstance);
    //     sphereInstance->set_mesh(sphere_mesh);
    //     sphereInstance->set_position(nodeCenter);
    //     sphereInstance->set_material_override(stitch_material);
    // }
}

void JarVoxelChunk::update_collision_mesh()
{
    // collision_shape->set_disabled(!chunk_mesh_data->has_collision_mesh);
    // concave_polygon_shape->set_data(chunk_mesh_data->collision_mesh);
}

void JarVoxelChunk::delete_chunk()
{
    // Implementation of UpdateDetailMeshes(0) not shown
}
