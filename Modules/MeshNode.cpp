#include "MeshNode.h"
#include <glm/gtc/matrix_transform.hpp>

MeshNode::MeshNode(unsigned int vao, int indexCount) : vao(vao), indexCount(indexCount) {}

void MeshNode::render(unsigned int shaderProgram, const glm::mat4& viewProjMatrix) {
    glm::mat4 world = getWorldTransform();
    glm::mat4 mvp = viewProjMatrix * world;

    int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glUniform3f(glGetUniformLocation(shaderProgram, "u_Color"), color.X, color.Y, color.Z);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Рендерим дочерние узлы
    Node::render(shaderProgram, viewProjMatrix);
}

public:
    Vector3 color = Vector3(1.0f, 0.6f, 0.1f);