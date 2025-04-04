#ifndef JAR_TERRAIN_POPULATOR_H
#define JAR_TERRAIN_POPULATOR_H

#include <algorithm>
#include <glm/glm.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <optional>


using namespace godot;

class JarTerrainPopulator : public Resource
{
    GDCLASS(JarTerrainPopulator, Resource);

  public:
    virtual ~JarTerrainPopulator() = default;    

  protected:
    static void _bind_methods()
    {
        // Binding methods for Godot        

    }
};

#endif // JAR_TERRAIN_POPULATOR_H
