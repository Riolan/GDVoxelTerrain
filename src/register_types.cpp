#include "register_types.h"
#include "jar_voxel_terrain.h"
#include "jar_voxel_chunk.h"
#include "jar_box_sdf.h"
#include "jar_sphere_sdf.h"
#include "jar_plane_sdf.h"
#include "jar_terrain_sdf.h"
#include "jar_planet_sdf.h"
#include "world.h"
#include "planar_world.h"
#include "spherical_world.h"
#include "terrain_populator.h"
#include "terrain_detail.h"
using namespace godot;

void initialize_jar_voxel_terrain_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        //SDFs
        GDREGISTER_ABSTRACT_CLASS(JarSignedDistanceField);
        GDREGISTER_CLASS(JarBoxSdf);
        GDREGISTER_CLASS(JarSphereSdf);
        GDREGISTER_CLASS(JarPlaneSdf);
        GDREGISTER_CLASS(JarTerrainSdf);
        GDREGISTER_CLASS(JarPlanetSdf);


        //TERRAIN
        GDREGISTER_CLASS(JarVoxelTerrain);
        GDREGISTER_CLASS(JarVoxelChunk);

        //WORLD
        GDREGISTER_ABSTRACT_CLASS(JarWorld);
        GDREGISTER_CLASS(JarPlanarWorld);
        GDREGISTER_CLASS(JarSphericalWorld);

        //POPULATION
        GDREGISTER_ABSTRACT_CLASS(JarTerrainPopulator);
        GDREGISTER_CLASS(JarTerrainDetail);

    }
}

void uninitialize_jar_voxel_terrain_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        
    }
}

extern "C"
{
    // Initialization.
    GDExtensionBool GDE_EXPORT jar_voxel_terrain_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                                const GDExtensionClassLibraryPtr p_library,
                                                                GDExtensionInitialization *r_initialization)
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_jar_voxel_terrain_module);
        init_obj.register_terminator(uninitialize_jar_voxel_terrain_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}