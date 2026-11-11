#include "MeshNode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

MeshNode::MeshNode(unsigned int vao, int indexCount) : vao(vao), indexCount(indexCount) {}

void MeshNode::render(unsigned int shaderProgram, const glm::mat4& viewProjMatrix) {
    // Container meshes (such as a mob's animation anchor) have no geometry.
    if (vao == 0 || indexCount <= 0) {
        Node::render(shaderProgram, viewProjMatrix);
        return;
    }
    glm::mat4 world = getWorldTransform();
    glm::mat4 mvp = viewProjMatrix * world;

    glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(world));
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "u_NormalMatrix"),
        1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_Model"), 1, GL_FALSE, &world[0][0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "u_HitFlash"), getHitFlash());

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
