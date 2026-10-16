#pragma once
#include "Node.h"
#include <glad/glad.h>

class MeshNode : public Node {
private:
    unsigned int vao;
    int indexCount;

public:
    MeshNode(unsigned int vao, int indexCount);

    void render(unsigned int shaderProgram, const glm::mat4& viewProjMatrix) override;
};