#include "jar_voxel_terrain.h"
#include "jar_plane_sdf.h"
#include "jar_sphere_sdf.h"
#include "modify_settings.h"

void JarVoxelTerrain::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_player_node"), &JarVoxelTerrain::get_player_node);
    ClassDB::bind_method(D_METHOD("set_player_node", "player_node"), &JarVoxelTerrain::set_player_node);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_node", PROPERTY_HINT_NODE_TYPE, "Node3D"), "set_player_node",
                 "get_player_node");

    ClassDB::bind_method(D_METHOD("get_octree_scale"), &JarVoxelTerrain::get_octree_scale);
    ClassDB::bind_method(D_METHOD("set_octree_scale", "value"), &JarVoxelTerrain::set_octree_scale);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "octree_scale"), "set_octree_scale", "get_octree_scale");

    ClassDB::bind_method(D_METHOD("get_size"), &JarVoxelTerrain::get_size);
    ClassDB::bind_method(D_METHOD("set_size", "value"), &JarVoxelTerrain::set_size);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "size"), "set_size", "get_size");

    ClassDB::bind_method(D_METHOD("get_min_chunk_size"), &JarVoxelTerrain::get_min_chunk_size);
    ClassDB::bind_method(D_METHOD("set_min_chunk_size", "value"), &JarVoxelTerrain::set_min_chunk_size);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "min_chunk_size"), "set_min_chunk_size", "get_min_chunk_size");

    ClassDB::bind_method(D_METHOD("get_lod"), &JarVoxelTerrain::get_lod);
    ClassDB::bind_method(D_METHOD("set_lod", "lod"), &JarVoxelTerrain::set_lod);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lod", PROPERTY_HINT_RESOURCE_TYPE, "JarVoxelLoD"), "set_lod",
                 "get_lod");

    ClassDB::bind_method(D_METHOD("get_chunk_scene"), &JarVoxelTerrain::get_chunk_scene);
    ClassDB::bind_method(D_METHOD("set_chunk_scene", "value"), &JarVoxelTerrain::set_chunk_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chunk_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_chunk_scene", "get_chunk_scene");

    ClassDB::bind_method(D_METHOD("get_sdf"), &JarVoxelTerrain::get_sdf);
    ClassDB::bind_method(D_METHOD("set_sdf", "sdf"), &JarVoxelTerrain::set_sdf);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sdf", PROPERTY_HINT_RESOURCE_TYPE, "JarSignedDistanceField"), "set_sdf",
                 "get_sdf");

    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_UNION);
    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_SUBTRACTION);
    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_INTERSECTION);
    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_SMOOTH_UNION);
    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_SMOOTH_SUBTRACTION);
    BIND_ENUM_CONSTANT(SDF::SDF_OPERATION_SMOOTH_INTERSECTION);
    ClassDB::bind_method(D_METHOD("modify", "sdf", "operation", "position", "radius"), &JarVoxelTerrain::modify);
    ClassDB::bind_method(D_METHOD("sphere_edit", "position", "radius", "union"), &JarVoxelTerrain::sphere_edit);
    ClassDB::bind_method(D_METHOD("spawn_debug_spheres_in_bounds", "position", "range"),
                         &JarVoxelTerrain::spawn_debug_spheres_in_bounds);
}

JarVoxelTerrain::JarVoxelTerrain() : _octreeScale(1.0f), _size(14), _playerNode(nullptr)
{
    _chunkSize = (1 << _minChunkSize);
}

void JarVoxelTerrain::modify(const Ref<JarSignedDistanceField> sdf, const SDF::Operation operation,
                             const Vector3 &position, const float radius)
{
    glm::vec3 pos = glm::vec3(position.x, position.y, position.z);
    auto s = new JarSphereSdf();
    s->set_radius(radius);

    auto edge = glm::vec3(radius);
    _modifySettingsQueue.push({s, Bounds(pos - edge, pos + edge), pos, operation});
}

void JarVoxelTerrain::sphere_edit(const Vector3 &position, const float radius, bool operation_union)
{
    glm::vec3 pos = glm::vec3(position.x, position.y, position.z);
    auto operation =
        operation_union ? SDF::Operation::SDF_OPERATION_SMOOTH_UNION : SDF::Operation::SDF_OPERATION_SMOOTH_SUBTRACTION;
    Ref<JarSphereSdf> sdf;
    sdf.instantiate();
    sdf->set_radius(radius);
    auto edge = glm::vec3(radius + _octreeScale * 2.0f);

    if (_isBuilding)
        return;
    ModifySettings settings = {sdf, Bounds(pos - edge, pos + edge), pos, operation};
    _voxelRoot->modify_sdf_in_bounds(this, settings);
    //_populationRoot->remove_population(settings);
    //_modifySettingsQueue.push({sdf, Bounds(pos - edge, pos + edge), pos, operation});
}

Node3D *JarVoxelTerrain::get_player_node() const
{
    return _playerNode;
}

void JarVoxelTerrain::set_player_node(Node3D *playerNode)
{
    _playerNode = playerNode;
}

bool JarVoxelTerrain::is_building() const
{
    return _isBuilding;
}

Ref<JarVoxelLoD> JarVoxelTerrain::get_lod() const
{
    return _voxelLod;
}
void JarVoxelTerrain::set_lod(const Ref<JarVoxelLoD> &lod)
{
    _voxelLod = lod;
}

Ref<JarSignedDistanceField> JarVoxelTerrain::get_sdf() const
{
    return _sdf;
}
void JarVoxelTerrain::set_sdf(const Ref<JarSignedDistanceField> &sdf)
{
    _sdf = sdf;
}

MeshComputeScheduler *JarVoxelTerrain::get_mesh_scheduler() const
{
    return _meshComputeScheduler;
}

float JarVoxelTerrain::get_octree_scale() const
{
    return _octreeScale;
}

void JarVoxelTerrain::set_octree_scale(float value)
{
    _octreeScale = value;
}

int JarVoxelTerrain::get_size() const
{
    return _size;
}

void JarVoxelTerrain::set_size(int value)
{
    _size = value;
}

int JarVoxelTerrain::get_min_chunk_size() const
{
    return _minChunkSize;
}

void JarVoxelTerrain::set_min_chunk_size(int value)
{
    _minChunkSize = value;
}

int JarVoxelTerrain::get_chunk_size() const
{
    return _chunkSize;
}

Ref<PackedScene> JarVoxelTerrain::get_chunk_scene() const
{
    return _chunkScene;
}

void JarVoxelTerrain::set_chunk_scene(const Ref<PackedScene> &value)
{
    _chunkScene = value;
}

void JarVoxelTerrain::_notification(int p_what)
{
    if (godot::Engine::get_singleton()->is_editor_hint())
    {
        return;
    }
    switch (p_what)
    {
    case NOTIFICATION_ENTER_TREE: {
        if (!Engine::get_singleton()->is_editor_hint())
        {
            set_process_internal(true);
        }
        else
        {
            set_process_internal(false);
        }
        break;
    }
    case NOTIFICATION_READY: {
        initialize();
        break;
    }
    case NOTIFICATION_EXIT_TREE: {
        set_process_internal(false);
        break;
    }
    case NOTIFICATION_INTERNAL_PROCESS: {
        process();
        break;
    }
    }
}

void JarVoxelTerrain::initialize()
{
    if (_voxelLod.is_null())
    {
        UtilityFunctions::printerr("No LoD properties, please provide them.");
        return;
    }
    if (_chunkScene.is_null())
    {
        UtilityFunctions::printerr("No ChunkScene properties, please provide it.");
        return;
    }
    if (_sdf.is_null())
    {
        UtilityFunctions::printerr("No sdf, please provide it.");
        return;
    }

    UtilityFunctions::print("Start");
    _chunkSize = (1 << _minChunkSize);
    _voxelLod->init();
    _meshComputeScheduler = new MeshComputeScheduler(12);
    _voxelRoot = new VoxelOctreeNode(_size);
    //_populationRoot = memnew(PopulationOctreeNode(_size));
    build();
}

void JarVoxelTerrain::process()
{
    if (_voxelLod->process(*this, 0.0))
        build();
    _meshComputeScheduler->process(this);

    if (!_modifySettingsQueue.empty())
    {
        process_modify_queue();
    }

    // process_chunk_queue(static_cast<float>(delta));
}

void printUniqueLoDValues(const std::vector<int> &lodValues)
{
    godot::String lodString = "Unique LoD values: ";
    for (const auto &lod : lodValues)
    {
        lodString += godot::String::num_int64(lod) + ", ";
    }
    // Remove the last comma and space
    if (lodValues.size() > 0)
    {
        lodString = lodString.substr(0, lodString.length() - 2);
    }
    UtilityFunctions::print(lodString);
}

void JarVoxelTerrain::build()
{
    if (_isBuilding)
        return;
    std::thread([this]() {
        _isBuilding = true;

        //_meshComputeScheduler->clear_queue();
        _voxelRoot->build(this);
        _isBuilding = false;
    }).detach();

    // std::thread([this]() { _worldBiomes->update_texture(_levelOfDetail->get_camera_position()); }).detach();
    // UtilityFunctions::print("Done Building.");
    // UtilityFunctions::print(_voxelRoot->get_count());

    // std::vector<int> uniqueLoDValues;
    // _voxelRoot->populateUniqueLoDValues(uniqueLoDValues);
    // printUniqueLoDValues(uniqueLoDValues);
}

void JarVoxelTerrain::process_chunk_queue(float delta)
{
    // if (_updateChunkCollidersQueue.empty())
    //     return;

    // int chunksPerSecond = _updateChunkCollidersQueue.size();
    // float max = std::max(chunksPerSecond * delta, 1.0f);

    // for (int i = 0; i < std::min(max, static_cast<float>(_updateChunkCollidersQueue.size())); i++)
    // {
    //     Chunk *chunk = _updateChunkCollidersQueue.front();
    //     _updateChunkCollidersQueue.pop();
    //     chunk->update_collision_mesh();
    // }
}

void JarVoxelTerrain::generate_epsilons()
{
    int numElements = _size + 1;

    std::vector<int> sizes(numElements);
    for (int i = 0; i < numElements; i++)
    {
        sizes[i] = 1 << i;
    }

    _voxelEpsilons.resize(sizes.size());
    for (int i = 0; i < sizes.size(); i++)
    {
        int size = sizes[i];
        float x = size * _octreeScale;
        _voxelEpsilons[i] = 1.75f * x;
    }
}

void JarVoxelTerrain::process_modify_queue()
{
    if (_isBuilding)
        return;
    _isBuilding = true;
    std::thread([this]() {
        if (!_modifySettingsQueue.empty())
        {
            auto &settings = _modifySettingsQueue.front();
            _modifySettingsQueue.pop();
            _voxelRoot->modify_sdf_in_bounds(this, settings);
            //_populationRoot->remove_population(settings);
        }
        _isBuilding = false;
    }).detach();
}

void JarVoxelTerrain::get_voxel_leaves_in_bounds(const Bounds &bounds, std::vector<VoxelOctreeNode *> &nodes) const
{
    _voxelRoot->get_voxel_leaves_in_bounds(this, bounds, nodes);
}

void JarVoxelTerrain::spawn_debug_spheres_in_bounds(const Vector3 &position, const float range)
{
    std::vector<VoxelOctreeNode *> nodes;
    auto center = glm::vec3(position.x, position.y, position.z);
    auto bounds = Bounds(center - range, center + range);
    get_voxel_leaves_in_bounds(bounds, nodes);

    Ref<StandardMaterial3D> red_material;
    red_material.instantiate();
    red_material->set_albedo(Color(1, 0, 0));
    Ref<StandardMaterial3D> green_material;
    green_material.instantiate();
    green_material->set_albedo(Color(0, 1, 0));

    Ref<SphereMesh> sphere_mesh;
    sphere_mesh.instantiate();
    sphere_mesh->set_radius(0.1f);
    sphere_mesh->set_height(0.2f);

    for (auto &&n : nodes)
    {
        Vector3 nodeCenter(n->_center.x, n->_center.y, n->_center.z);

        MeshInstance3D *sphereInstance = memnew(MeshInstance3D);
        add_child(sphereInstance);

        sphereInstance->set_mesh(sphere_mesh);
        sphereInstance->set_position(nodeCenter);
        sphereInstance->set_material_override((n->_value > 0) ? green_material : red_material);
    }
}
