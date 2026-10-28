#pragma once
#include "Vector3.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>


class Node : public std::enable_shared_from_this<Node> {
protected:
    Node* parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;

    Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 rotation = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
    Vector3 visualOffset;
    float hitFlash = 0.f;

    glm::mat4 cachedWorldMatrix = glm::mat4(1.0f);
    bool isTransformDirty = true;

public:
    virtual ~Node() = default;

    virtual void _Process(float deltaTime) {
    }

    void addChild(std::shared_ptr<Node> child) {
        child->parent = this;
        children.push_back(child);
    }

    void setPosition(const Vector3& pos) { position = pos; isTransformDirty = true; }
    void setRotation(const Vector3& rot) { rotation = rot; isTransformDirty = true; }
    void setScale(const Vector3& scl) { scale = scl; isTransformDirty = true; }
    void setVisualOffset(const Vector3& offset) { visualOffset = offset; isTransformDirty = true; }
    void setHitFlash(float amount) { hitFlash = amount; }
    float getHitFlash() const { return std::max(hitFlash, parent ? parent->getHitFlash() : 0.f); }

    Vector3 getPosition() const { return position; }
    Vector3 getRotation() const { return rotation; }
    Vector3 getScale() const { return scale; }

    glm::mat4 getLocalTransform() const {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::translate(mat, position.toGlm());
        mat = glm::translate(mat, visualOffset.toGlm());
        mat = glm::rotate(mat, glm::radians(rotation.X), glm::vec3(1, 0, 0));
        mat = glm::rotate(mat, glm::radians(rotation.Y), glm::vec3(0, 1, 0));
        mat = glm::rotate(mat, glm::radians(rotation.Z), glm::vec3(0, 0, 1));
        mat = glm::scale(mat, scale.toGlm());
        return mat;
    }

    glm::mat4 getWorldTransform() {
        if (isTransformDirty || parent != nullptr) {
            glm::mat4 local = getLocalTransform();
            if (parent != nullptr) 
                cachedWorldMatrix = parent->getWorldTransform() * local;
            else 
                cachedWorldMatrix = local;
            
            isTransformDirty = false;
        }
        return cachedWorldMatrix;
    }

    virtual void update(float deltaTime) {
        _Process(deltaTime);
        isTransformDirty = true;
        for (auto& child : children) {
            child->update(deltaTime);
        }
    }

    virtual void render(GLuint shaderProgram, const glm::mat4& viewProjMatrix) {
        for (auto& child : children) {
            child->render(shaderProgram, viewProjMatrix);
        }
    }
};
