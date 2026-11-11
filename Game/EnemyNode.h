#pragma once
#include "Battle.h"
#include "../Modules/MeshNode.h"
#include <array>
#include "DissolveState.h"

class EnemyNode : public Node {
public:
    Fighter stats;
    std::shared_ptr<MeshNode> body;

    EnemyNode(GLuint cubeVao, int cubeIndexCount);
    void spawn(int typeIndex, float difficulty, Vector3 position);
    void keepVisibleForFeedback(bool value) { keepDefeatedVisible = value; }
    void _Process(float dt) override;
    void setDissolveProgram(GLuint effect, GLuint depth) { dissolveProgram = effect; depthProgram = depth; }
    bool dissolveFinished() const { return dissolve.finished(); }
    float dissolveProgress() const { return dissolve.progress(); }
    void render(GLuint shaderProgram, const glm::mat4& viewProjection) override;

private:
    GLuint cubeVao;
    int cubeIndexCount;
    bool keepDefeatedVisible = false;
    DissolveState dissolve;
    GLuint dissolveProgram = 0, depthProgram = 0;
    std::shared_ptr<MeshNode> hpBar;
    std::array<std::shared_ptr<Node>, 3> models;

    void buildModels();
    std::shared_ptr<MeshNode> addBlock(const std::shared_ptr<Node>& parent,
        Vector3 position, Vector3 scale, Vector3 color);
};
