#ifndef VOXEL_TERRAIN_H
#define VOXEL_TERRAIN_H

#include "jar_signed_distance_field.h"
#include "mesh_compute_scheduler.h"
#include "modify_settings.h"
#include "voxel_lod.h"
#include "voxel_octree_node.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <queue>
#include <vector>

using namespace godot;

class JarVoxelTerrain : public Node3D
{
    GDCLASS(JarVoxelTerrain, Node3D);

  public:
    MeshComputeScheduler *_meshComputeScheduler;
    VoxelLoD *_levelOfDetail;

    std::vector<float> _voxelEpsilons;

    Ref<JarSignedDistanceField> _sdf;
    VoxelOctreeNode *_voxelRoot;

    std::queue<ModifySettings> _modifySettingsQueue;

    // Exported variables
    float _octreeScale = 1.0f;
    int _size = 8;
    int _minChunkSize = 4; // each chunk is 2^4 = 16*16*16 voxels

    bool _updateLoDAutomatically = true;
    float _automaticUpdateDistance = 64.0f;
    int _lodCount = 8;
    float _lodDistance = 128.0f;
    float _lodPadding = 0.0f;

    Ref<PackedScene> _chunkScene;

    bool _isBuilding = false;
    int _chunkSize = 0;

    void build();
    void _notification(int what);
    void initialize();
    void process();
    void process_chunk_queue(float delta);
    void generate_epsilons();
    void process_modify_queue();

  protected:
    static void _bind_methods();

  public:
    JarVoxelTerrain();

    //enum
    void modify(const Ref<JarSignedDistanceField> sdf, const SDF::Operation operation, const Vector3 &position, const float radius);
    void sphere_edit(const Vector3 &position, const float radius, bool operation_union);

    // properties
    bool is_building() const;
    VoxelLoD *get_lod() const;
    MeshComputeScheduler *get_mesh_scheduler() const;

    // properties

    Ref<JarSignedDistanceField> get_sdf() const;
    void set_sdf(const Ref<JarSignedDistanceField> &sdf);

    float get_octree_scale() const;
    void set_octree_scale(float value);

    int get_size() const;
    void set_size(int value);

    int get_min_chunk_size() const;
    void set_min_chunk_size(int value);

    int get_chunk_size() const;

    bool get_update_lod_automatically() const;
    void set_update_lod_automatically(bool value);

    float get_automatic_update_distance() const;
    void set_automatic_update_distance(float value);

    int get_lod_count() const;
    void set_lod_count(int value);

    float get_lod_distance() const;
    void set_lod_distance(float value);

    float get_lod_padding() const;
    void set_lod_padding(float value);

    Ref<Mesh> get_mesh() const;
    void set_mesh(const Ref<Mesh> &value);

    Ref<PackedScene> get_chunk_scene() const;
    void set_chunk_scene(const Ref<PackedScene> &value);
    
    void get_voxel_leaves_in_bounds(const Bounds &bounds, std::vector<VoxelOctreeNode *> &nodes) const;

    void spawn_debug_spheres_in_bounds(const Vector3 &position, const float range);
};


VARIANT_ENUM_CAST(SDF::Operation);

#endif // VOXEL_TERRAIN_H
