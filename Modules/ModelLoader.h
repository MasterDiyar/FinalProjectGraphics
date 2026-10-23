#pragma once
#include <string>

struct Model {
    unsigned int vao = 0;
    int indexCount = 0;
};

// pos(3) normal(3) uv(2), location 0/1/2, stride 8 floats
Model loadOBJ(const std::string& path);