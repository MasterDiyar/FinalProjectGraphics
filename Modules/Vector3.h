#pragma once
#include <glm/glm.hpp>

class Vector3
{
public:
    float X, Y, Z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : X(x), Y(y), Z(z) {}

    glm::vec3 toGlm() const {
        return glm::vec3(X, Y, Z);
    }
};