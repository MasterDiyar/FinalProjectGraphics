#pragma once
#include "Node.h"
#include "MeshNode.h"
#include "ModelLoader.h"
#include "../Game/Battle.h"
#include <memory>
#include <string>

class PlayerNode : public Node {
private:
    std::shared_ptr<MeshNode> bodyNode;
    float moveSpeed = 4.0f;
    float arenaLimit = 9.5f;

public:
    Fighter stats{ "Hero", 40, 40, 9, 2, 10, 10 };

    PlayerNode(unsigned int cubeVAO, int cubeIndexCount);

    void loadModel(const std::string& objPath, const std::string& texturePath, unsigned int cubeVAO, int cubeIndexCount);

    void handleMovement(const bool keys[256], float dt);

    // Доступ к телу персонажа для анимаций атак/ударов
    std::shared_ptr<MeshNode> getBody() const { return bodyNode; }
};