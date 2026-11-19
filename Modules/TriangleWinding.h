#pragma once
#include <algorithm>
#include <glm/glm.hpp>
// Match each indexed triangle to the outward normal stored in its vertices.
inline void orientTriangles(const float* vertices, unsigned int* indices, int count) {
    for (int i = 0; i < count; i += 3) {
        auto point = [&](unsigned int index) { return glm::vec3(vertices[index*8], vertices[index*8+1], vertices[index*8+2]); };
        const unsigned int a = indices[i];
        glm::vec3 normal(vertices[a*8+3], vertices[a*8+4], vertices[a*8+5]);
        if (glm::dot(glm::cross(point(indices[i+1])-point(a), point(indices[i+2])-point(a)), normal) < 0.f)
            std::swap(indices[i+1], indices[i+2]);
    }
}
