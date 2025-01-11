#ifndef SDF_OPERATIONS_H
#define SDF_OPERATIONS_H

#include <algorithm>
#include <glm/glm.hpp>

class SdfOperation {
public:
    virtual float apply(float a, float b, float k= 1.0f) const = 0;
};

class UnionOperation : public SdfOperation {
public:
    float apply(float a, float b, float k) const override {
        return std::min(a, b);
    }
};

class IntersectionOperation : public SdfOperation {
public:
    float apply(float a, float b, float k) const override {
        return std::max(a, b);
    }
};

class SubtractionOperation : public SdfOperation {
public:
    float apply(float a, float b, float k) const override {
        return std::max(a, -b);
    }
};

class SmoothUnionOperation : public SdfOperation {
public:
    float apply(float d1, float d2, float k) const override {
        float h = std::clamp(0.5f + 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
        return glm::mix(d2, d1, h) - k * h * (1.0f - h);
    }
};

class SmoothSubtractionOperation : public SdfOperation {
public:
    float apply(float d1, float d2, float k) const override {
        float h = std::clamp(0.5f - 0.5f * (d2 + d1) / k, 0.0f, 1.0f);
        return glm::mix(d1, -d2, h) + k * h * (1.0f - h);
    }
};

class SmoothIntersectionOperation : public SdfOperation {
public:
    float apply(float d1, float d2, float k) const override {
        float h = std::clamp(0.5f - 0.5f * (d2 - d1) / k, 0.0f, 1.0f);
        return glm::mix(d2, d1, h) + k * h * (1.0f - h);
    }
};

#endif // SDF_OPERATIONS_H
