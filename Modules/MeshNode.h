#pragma once
#include "Node.h"
#include "Vector3.h"

class MeshNode : public Node {
public:
    Vector3 color = Vector3(1.0f, 0.6f, 0.1f);
    unsigned int textureId = 0;  

    unsigned int vao;
    int indexCount;

    MeshNode(unsigned int vao, int indexCount);
    void render(unsigned int shaderProgram, const glm::mat4& viewProjMatrix) override;
};