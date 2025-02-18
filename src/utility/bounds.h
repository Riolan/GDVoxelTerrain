#ifndef JAR_AABB_H
#define JAR_AABB_H

#include <glm/glm.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/vector3.hpp>

struct Bounds {
public:
    glm::vec3 min;
    glm::vec3 max;

    Bounds() : min(0.0f), max(0.0f) {}
    
    Bounds(const glm::vec3 &min, const glm::vec3 &max) {
        this->min = glm::min(min, max);
        this->max = glm::max(min, max);
    }

    Bounds(const godot::AABB &aabb) {
        godot::Vector3 godot_min = aabb.position;
        godot::Vector3 godot_max = aabb.position + aabb.size;
        min = {godot_min.x, godot_min.y, godot_min.z};
        max = {godot_max.x, godot_max.y, godot_max.z};
        // Ensure validity
        min = glm::min(min, max);
        max = glm::max(min, max);
    }

    inline bool is_valid() const {
        return (min.x <= max.x && min.y <= max.y && min.z <= max.z);
    }

    inline glm::vec3 get_center() const {
        return (min + max) * 0.5f;
    }

    inline glm::vec3 get_size() const {
        return max - min;
    }

    inline bool contains_point(const glm::vec3 &point) const {
        return (point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z);
    }

    inline Bounds intersected(const Bounds &other) const {
        glm::vec3 new_min = glm::max(min, other.min);
        glm::vec3 new_max = glm::min(max, other.max);
        if (new_min.x > new_max.x || new_min.y > new_max.y || new_min.z > new_max.z) {
            return Bounds(); // Empty bounds
        }
        return Bounds(new_min, new_max);
    }

    inline Bounds expanded(const glm::vec3 &amount) const {
        return Bounds(min - amount, max + amount);
    }

    inline Bounds expanded(float amount) const {
        return expanded(glm::vec3(amount));
    }

    inline bool intersects(const Bounds &other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z);
    }

    inline bool encloses(const Bounds &other) const {
        return (min.x <= other.min.x && max.x >= other.max.x &&
                min.y <= other.min.y && max.y >= other.max.y &&
                min.z <= other.min.z && max.z >= other.max.z);
    }
};

#endif // JAR_AABB_H