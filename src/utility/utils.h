#ifndef UTILS_H
#define UTILS_H


#ifdef _MSC_VER
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE __attribute__((always_inline)) inline
#endif


#include "bounds.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/string.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <vector>

namespace godot
{

class Utils : public Object
{
    GDCLASS(Utils, Object);

  public:
    Utils()
    {
    }
    ~Utils()
    {
    }

    static PackedFloat32Array vector_to_array_float(const std::vector<float> &vec)
    {
        PackedFloat32Array array;
        array.resize(vec.size());
        std::copy(vec.begin(), vec.end(), array.ptrw());
        return array;
    }

    static std::vector<float> array_to_vector_float(const PackedFloat32Array &array)
    {
        std::vector<float> vec(array.size());
        std::copy(array.ptr(), array.ptr() + array.size(), vec.begin());
        return vec;
    }

    static PackedInt32Array vector_to_array_int(const std::vector<int> &vec)
    {
        PackedInt32Array array;
        array.resize(vec.size());
        std::copy(vec.begin(), vec.end(), array.ptrw());
        return array;
    }

    static std::vector<int> array_to_vector_int(const PackedInt32Array &array)
    {
        std::vector<int> vec(array.size());
        std::copy(array.ptr(), array.ptr() + array.size(), vec.begin());
        return vec;
    }

    static godot::String vector_to_string_float(const std::vector<float> &vec)
    {
        godot::String string = "[";
        for (const float &f : vec)
        {
            string += String::num(f, 2) + ", ";
        }
        return string + "]";
    }

    static godot::String to_string(const Bounds &bounds)
    {
        return godot::String(("{" + glm::to_string(bounds.min) + ", " + glm::to_string(bounds.max) + "}").c_str());
    }

    template<class matType>
    static godot::String to_string(const matType &v)
    {
        return godot::String(("{" + glm::to_string(v) + "}").c_str());
    }
};

} // namespace godot

#endif
