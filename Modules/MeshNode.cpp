#include "MeshNode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

MeshNode::MeshNode(unsigned int vao, int indexCount) : vao(vao), indexCount(indexCount) {}

void MeshNode::render(unsigned int shaderProgram, const glm::mat4& viewProjMatrix) {
    glm::mat4 world = getWorldTransform();
    glm::mat4 mvp = viewProjMatrix * world;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Model"), 1, GL_FALSE, &world[0][0]);

    int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glUniform3f(glGetUniformLocation(shaderProgram, "u_Color"), color.X, color.Y, color.Z);

    glUniform1i(glGetUniformLocation(shaderProgram, "u_UseTex"), textureId != 0);
    if (textureId != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Рендерим дочерние узлы
    Node::render(shaderProgram, viewProjMatrix);
}
