#ifndef JAR_AABB_H
#define JAR_AABB_H

#include <glm/glm.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3.hpp>

struct Bounds
{
  public:
    glm::vec3 min;
    glm::vec3 max;

    Bounds() : min(glm::vec3(0.0f)), max(glm::vec3(0.0f))
    {
    }
    Bounds(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max)
    {
    }
    Bounds(const godot::AABB &aabb)
    {
        godot::Vector3 e = aabb.get_end();
        min = {aabb.position.x, aabb.position.y, aabb.position.z}; 
        max = {e.x, e.y, e.z};
    }

    inline glm::vec3 get_center() const
    {
        return (min + max) * 0.5f;
    }

    inline glm::vec3 get_size() const
    {
        return max - min;
    }

    inline bool contains_point(const glm::vec3 &point) const
    {
        return (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z &&
                point.z <= max.z);
    }

    inline Bounds intersected(const Bounds &other) const
    {
        return Bounds(glm::max(min, other.min), glm::min(max, other.max));
    }

    inline Bounds expanded(const glm::vec3 &amount) const
    {
        return Bounds(min - amount, max + amount);
    }

    inline Bounds expanded(const float amount) const
    {
        return Bounds(min - amount, max + amount);
    }

    inline bool intersects(const Bounds &other) const
    {
        return (min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z);
    }

    inline bool encloses(const Bounds &other) const
    {
        return (min.x <= other.min.x && max.x >= other.max.x && min.y <= other.min.y && max.y >= other.max.y &&
                min.z <= other.min.z && max.z >= other.max.z);
    }
};

#endif // JAR_AABB_H
