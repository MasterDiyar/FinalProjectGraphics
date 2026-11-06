#include "PlayerNode.h"
#include "PlayerNode.h"
#include "Texture.h"
#include <cmath>
#include <algorithm>

PlayerNode::PlayerNode(unsigned int cubeVAO, int cubeIndexCount) {
    loadModel("assets/hero.obj", "assets/hero.png", cubeVAO, cubeIndexCount);
}

void PlayerNode::loadModel(const std::string& objPath, const std::string& texturePath, unsigned int cubeVAO, int cubeIndexCount) {
    Model heroModel = loadOBJ(objPath);

    if (heroModel.vao != 0) {
        bodyNode = std::make_shared<MeshNode>(heroModel.vao, heroModel.indexCount);
        bodyNode->textureId = loadTexture(texturePath);
        bodyNode->color = Vector3(1.0f, 1.0f, 1.0f);
    }
    else {
        // Запасной кубический игрок, если OBJ не загрузился
        bodyNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        bodyNode->color = Vector3(0.2f, 0.5f, 1.0f);

        // Голова
        auto head = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        head->setPosition(Vector3(0, 0.83f, 0));
        head->setScale(Vector3(0.65f, 0.65f, 0.65f));
        head->color = Vector3(1.f, 0.79f, 0.53f);
        bodyNode->addChild(head);

        // Руки
        for (float x : {-0.65f, 0.65f}) {
            auto arm = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
            arm->setPosition(Vector3(x, 0, 0));
            arm->setScale(Vector3(0.25f, 0.8f, 0.4f));
            arm->color = bodyNode->color;
            bodyNode->addChild(arm);
        }

        // Глаза
        for (float x : {-0.15f, 0.15f}) {
            auto eye = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
            eye->setPosition(Vector3(x, 0.88f, 0.34f));
            eye->setScale(Vector3(0.09f, 0.1f, 0.03f));
            eye->color = Vector3(0.08f, 0.1f, 0.15f);
            bodyNode->addChild(eye);
        }
    }

    addChild(bodyNode);
}

void PlayerNode::handleMovement(const bool keys[256], float dt) {
    Vector3 p = getPosition();
    float dx = float(keys['d']) - float(keys['a']);
    float dz = float(keys['s']) - float(keys['w']);
    float length = std::sqrt(dx * dx + dz * dz);

    if (length > 0.0f) {
        p.X += (dx / length) * moveSpeed * dt;
        p.Z += (dz / length) * moveSpeed * dt;
    }

    // Ограничение движения границами арены
    p.X = std::clamp(p.X, -arenaLimit, arenaLimit);
    p.Z = std::clamp(p.Z, -arenaLimit, arenaLimit);

    setPosition(p);
}