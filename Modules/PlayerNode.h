#pragma once
#include "Node.h"
#include "MeshNode.h"
#include "ModelLoader.h"
#include "../Game/Battle.h"
#include <glad/glad.h>
#include <memory>
#include <string>
class PlayerNode : public Node {
private:
    std::shared_ptr<MeshNode> bodyNode;
    float moveSpeed = 4.0f;
    float arenaLimit = 9.5f;

    GLuint toonShaderProgram = 0;
    GLuint rimShaderProgram = 0;

    void initPlayerShaders();
    GLuint compileShader(GLenum type, const std::string& source);

public:
    Fighter stats{ "Hero", 40, 40, 9, 2, 10, 10 };

    PlayerNode(unsigned int cubeVAO, int cubeIndexCount);
    ~PlayerNode();

    void loadModel(const std::string& objPath, const std::string& texturePath, unsigned int cubeVAO, int cubeIndexCount);
    void handleMovement(const bool keys[256], float dt, float cameraAngle);

    std::shared_ptr<MeshNode> getBody() const { return bodyNode; }

    void render(GLuint shaderProgram, const glm::mat4& viewProjMatrix) override;

    void renderWithRim(GLuint defaultShaderProgram, const glm::mat4& vpMatrix, const glm::vec3& cameraPos);
};