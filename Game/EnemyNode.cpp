#include "EnemyNode.h"
#include <algorithm>
#include "ShadowMap.h"
#include <cmath>

namespace {
struct EnemyType {
    const char* name;
    int hp, atk, def;
    Vector3 color;
};

const EnemyType enemyTypes[] = {
    { "Slime",  20, 6, 1, Vector3(0.7f, 0.3f, 0.9f) },
    { "Goblin", 28, 8, 2, Vector3(0.9f, 0.5f, 0.1f) },
    { "Golem",  40, 7, 4, Vector3(0.5f, 0.5f, 0.55f) },
};

}

EnemyNode::EnemyNode(GLuint vao, int indexCount) : cubeVao(vao), cubeIndexCount(indexCount) {
    body = std::make_shared<MeshNode>(0, 0);
    buildModels();
    hpBar = std::make_shared<MeshNode>(cubeVao, cubeIndexCount);
    hpBar->color = Vector3(0.9f, 0.1f, 0.1f);
    addChild(body);
    addChild(hpBar);
}

std::shared_ptr<MeshNode> EnemyNode::addBlock(const std::shared_ptr<Node>& parent,
    Vector3 position, Vector3 scale, Vector3 color) {
    auto block = std::make_shared<MeshNode>(cubeVao, cubeIndexCount);
    block->setPosition(position);
    block->setScale(scale);
    block->color = color;
    parent->addChild(block);
    return block;
}

void EnemyNode::buildModels() {
    // Reuse the same encounter/animation anchor while switching child models.
    for (int type = 0; type < 3; ++type) {
        auto model = std::make_shared<Node>();
        models[type] = model;
        body->addChild(model);
        Vector3 color = enemyTypes[type].color;
        float eyeY = 0.05f, eyeZ = 0.52f;
        if (type == 0) {
            addBlock(model, Vector3(0, -0.18f, 0), Vector3(1.25f, 0.64f, 1.05f), color);
            addBlock(model, Vector3(0, 0.2f, -0.08f), Vector3(0.7f, 0.2f, 0.65f), color);
        }
        else if (type == 1) {
            addBlock(model, Vector3(0, -0.05f, 0), Vector3(0.65f, 0.8f, 0.5f), Vector3(0.38f, 0.25f, 0.14f));
            addBlock(model, Vector3(0, 0.62f, 0), Vector3(0.8f, 0.65f, 0.7f), color);
            for (float side : {-1.f, 1.f}) {
                auto ear = addBlock(model, Vector3(side * 0.55f, 0.73f, 0), Vector3(0.4f, 0.18f, 0.35f), color);
                ear->setRotation(Vector3(0, 0, side * 25.f));
                addBlock(model, Vector3(side * 0.48f, -0.02f, 0), Vector3(0.22f, 0.65f, 0.3f), color);
            }
            eyeY = 0.67f;
            eyeZ = 0.37f;
        }
        else {
            addBlock(model, Vector3(0, 0.1f, 0), Vector3(1.15f, 1.2f, 0.8f), color);
            addBlock(model, Vector3(0, 0.91f, 0), Vector3(0.7f, 0.5f, 0.65f), color);
            for (float side : {-1.f, 1.f}) {
                addBlock(model, Vector3(side * 0.79f, 0.05f, 0), Vector3(0.4f, 1.f, 0.65f), color);
                addBlock(model, Vector3(side * 0.35f, 0.78f, -0.15f), Vector3(0.23f, 0.3f, 0.4f), Vector3(0.32f, 0.35f, 0.4f));
            }
            eyeY = 0.95f;
            eyeZ = 0.35f;
        }
        for (float x : {-0.2f, 0.2f}) {
            addBlock(model, Vector3(x, eyeY, eyeZ), Vector3(0.2f, 0.22f, 0.04f), Vector3(0.95f, 0.95f, 0.86f));
            addBlock(model, Vector3(x, eyeY, eyeZ + 0.03f), Vector3(0.08f, 0.12f, 0.03f), Vector3(0.08f, 0.1f, 0.15f));
        }
        addBlock(model, Vector3(0, eyeY - 0.22f, eyeZ), Vector3(0.25f, 0.06f, 0.04f), Vector3(0.12f, 0.08f, 0.16f));
    }
}

void EnemyNode::spawn(int typeIndex, float difficulty, Vector3 position) {
    typeIndex = std::clamp(typeIndex, 0, 2);
    activeType = typeIndex;
    // Position-dependent phases keep different mobs from moving in lockstep.
    idleTime = std::fmod(std::abs(position.X * 0.37f + position.Z * 0.61f), 1.f);
    const EnemyType& type = enemyTypes[typeIndex];
    int hp = static_cast<int>(type.hp * difficulty);
    stats = Fighter{type.name, hp, hp, static_cast<int>(type.atk * difficulty), type.def, 0, 0};
    for (int i = 0; i < 3; ++i) {
        models[i]->setPosition(Vector3());
        models[i]->setRotation(Vector3());
        models[i]->setScale(i == typeIndex ? Vector3(1, 1, 1) : Vector3());
    }
    body->setPosition(position);
    body->setVisualOffset(Vector3());
    body->setHitFlash(0.f);
    keepDefeatedVisible = false;
    dissolve.reset();
}

void EnemyNode::_Process(float dt) {
    if (stats.alive() && dt > 0.f) {
        const float period = activeType == 0 ? 1.2f : activeType == 1 ? 1.7f : 2.4f;
        idleTime = std::fmod(idleTime + dt, period);
        float wave = std::sin(idleTime * 6.2831853f / period);
        auto& model = models[activeType];
        if (activeType == 0) {
            float height = 1.f - 0.10f * wave;
            model->setScale(Vector3(1.f + 0.06f * wave, height, 1.f + 0.06f * wave));
            model->setPosition(Vector3(0, 0.5f * (height - 1.f), 0));
        } else if (activeType == 1) {
            model->setRotation(Vector3(0, 0, 2.5f * wave));
            model->setPosition(Vector3(0, 0.025f * (1.f + wave), 0));
        } else {
            float height = 1.f + 0.025f * wave;
            model->setScale(Vector3(1.f, height, 1.f));
            model->setPosition(Vector3(0, 0.5f * (height - 1.f), 0));
        }
    }
    if (!stats.alive() && !keepDefeatedVisible) {
        dissolve.start();
        dissolve.update(dt);
    }
    bool visible = stats.alive() || keepDefeatedVisible || !dissolve.finished();
    body->setScale(visible ? Vector3(1, 1, 1) : Vector3());
    if (!stats.alive()) { hpBar->setScale(Vector3()); return; }
    float ratio = static_cast<float>(stats.hp) / stats.maxHp;
    hpBar->setScale(Vector3(std::max(ratio, 0.001f) * 1.5f, 0.15f, 0.15f));
    Vector3 p = body->getPosition();
    hpBar->setPosition(Vector3(p.X, p.Y + 1.6f, p.Z));
}

void EnemyNode::render(GLuint shaderProgram, const glm::mat4& viewProjection) {
    if (dissolve.finished()) return;
    if (dissolveProgram == 0 || dissolve.progress() == 0.f) {
        Node::render(shaderProgram, viewProjection);
        return;
    }
    glUseProgram(dissolveProgram);
    const glm::vec3 origin(body->getWorldTransform()[3]);
    const glm::vec3 sun = ShadowMap::sunDirection();
    const glm::mat4 lightVP = ShadowMap::sunMatrix();
    glUniform3fv(glGetUniformLocation(dissolveProgram, "u_DissolveOrigin"), 1, &origin[0]);
    glUniform1f(glGetUniformLocation(dissolveProgram, "u_DissolveProgress"), dissolve.progress());
    glUniform1i(glGetUniformLocation(dissolveProgram, "u_DepthOnly"), shaderProgram == depthProgram);
    glUniform3fv(glGetUniformLocation(dissolveProgram, "u_SunDir"), 1, &sun[0]);
    glUniformMatrix4fv(glGetUniformLocation(dissolveProgram, "u_LightVP"), 1, GL_FALSE, &lightVP[0][0]);
    glUniform1i(glGetUniformLocation(dissolveProgram, "u_ShadowMap"), 1);
    glUniform1i(glGetUniformLocation(dissolveProgram, "u_Tex"), 0);
    body->render(dissolveProgram, viewProjection);
    glUseProgram(shaderProgram);
}
