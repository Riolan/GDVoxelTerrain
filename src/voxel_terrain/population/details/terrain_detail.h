#ifndef JAR_TERRAIN_DETAIL_H
#define JAR_TERRAIN_DETAIL_H

#include <algorithm>
#include <glm/glm.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <optional>


using namespace godot;

class JarTerrainDetail : public Resource
{
    GDCLASS(JarTerrainDetail, Resource);

  public:
    virtual ~JarTerrainDetail() = default;    

  protected:
    static void _bind_methods()
    {
        // Binding methods for Godot        

    }
};

#endif // JAR_TERRAIN_DETAIL_H
