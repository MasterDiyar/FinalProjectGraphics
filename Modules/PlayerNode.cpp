#include "PlayerNode.h"
#include "Texture.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static std::string loadShaderFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

PlayerNode::PlayerNode(unsigned int cubeVAO, int cubeIndexCount) {
    initPlayerShaders();
    loadModel("assets/hero.obj", "assets/hero.png", cubeVAO, cubeIndexCount);
}

PlayerNode::~PlayerNode() {
    if (toonShaderProgram) glDeleteProgram(toonShaderProgram);
    if (rimShaderProgram) glDeleteProgram(rimShaderProgram);
}

GLuint PlayerNode::compileShader(GLenum type, const std::string& source) {
    const char* src = source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compilation failed: ") + log);
    }
    return shader;
}

void PlayerNode::initPlayerShaders() {
    try {
        /*
        std::string toonVertCode = loadShaderFile("shaders/player_shader.vert");
        std::string toonFragCode = loadShaderFile("shaders/player_shader.frag");

        GLuint tvs = compileShader(GL_VERTEX_SHADER, toonVertCode);
        GLuint tfs = compileShader(GL_FRAGMENT_SHADER, toonFragCode);
        toonShaderProgram = glCreateProgram();
        glAttachShader(toonShaderProgram, tvs);
        glAttachShader(toonShaderProgram, tfs);
        glLinkProgram(toonShaderProgram);
        glDeleteShader(tvs);
        glDeleteShader(tfs);*/

        // Загрузка Rim Lighting шейдера
        std::string rimVertCode = loadShaderFile("shaders/player_rim.vert");
        std::string rimFragCode = loadShaderFile("shaders/player_rim.frag");

        GLuint rvs = compileShader(GL_VERTEX_SHADER, rimVertCode);
        GLuint rfs = compileShader(GL_FRAGMENT_SHADER, rimFragCode);
        rimShaderProgram = glCreateProgram();
        glAttachShader(rimShaderProgram, rvs);
        glAttachShader(rimShaderProgram, rfs);
        glLinkProgram(rimShaderProgram);
        glDeleteShader(rvs);
        glDeleteShader(rfs);
    }
    catch (const std::exception& e) {
        std::cerr << "Shader loading error: " << e.what() << std::endl;
    }
}

void PlayerNode::loadModel(const std::string& objPath, const std::string& texturePath, unsigned int cubeVAO, int cubeIndexCount) {
    Model heroModel = loadOBJ(objPath);

    if (heroModel.vao != 0) {
        bodyNode = std::make_shared<MeshNode>(heroModel.vao, heroModel.indexCount);
        bodyNode->textureId = loadTexture(texturePath);
        bodyNode->color = Vector3(1.0f, 1.0f, 1.0f);
        //bodyNode->setScale(Vector3(0.01f, 0.01f, 0.01f));
    }
    else {
        bodyNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        bodyNode->color = Vector3(0.2f, 0.5f, 1.0f);

        auto head = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        head->setPosition(Vector3(0, 0.83f, 0));
        head->setScale(Vector3(0.65f, 0.65f, 0.65f));
        head->color = Vector3(1.f, 0.79f, 0.53f);
        bodyNode->addChild(head);

        for (float x : {-0.65f, 0.65f}) {
            auto arm = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
            arm->setPosition(Vector3(x, 0, 0));
            arm->setScale(Vector3(0.25f, 0.8f, 0.4f));
            arm->color = bodyNode->color;
            bodyNode->addChild(arm);
        }

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

void PlayerNode::handleMovement(const bool keys[256], float dt, float cameraAngle) {
    Vector3 p = getPosition();
    float rawDx = float(keys['d']) - float(keys['a']);
    float dz = float(keys['s']) - float(keys['w']);
    float length = std::sqrt(rawDx * rawDx + dz * dz);

    if (length > 0.0f) {
        float inputX = rawDx / length;
        float inputZ = dz / length;

        float cosA = std::cos(cameraAngle);
        float sinA = std::sin(cameraAngle);

        float finalDx = inputX * cosA - inputZ * sinA;
        float finalDz = inputX * sinA + inputZ * cosA;

        p.X += finalDx * moveSpeed * dt;
        p.Z += finalDz * moveSpeed * dt;
    }

    p.X = std::clamp(p.X, -arenaLimit, arenaLimit);
    p.Z = std::clamp(p.Z, -arenaLimit, arenaLimit);

    setPosition(p);
}

void PlayerNode::render(GLuint shaderProgram, const glm::mat4& viewProjMatrix) {
    GLuint activeShader = (rimShaderProgram != 0) ? rimShaderProgram : shaderProgram;

    glUseProgram(activeShader);

    if (activeShader == rimShaderProgram) {
        glUniform3f(glGetUniformLocation(rimShaderProgram, "u_RimColor"), 0.1f, 0.35f, 0.8f);
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_RimPower"), 4.5f);
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_RimIntensity"), 0.21f);
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_HitFlash"), getHitFlash());
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_Alpha"), 1.0f);
    }

    Node::render(activeShader, viewProjMatrix);
    glUseProgram(shaderProgram);
}

void PlayerNode::renderWithRim(GLuint defaultShaderProgram, const glm::mat4& vpMatrix, const glm::vec3& cameraPos) {
    GLuint shaderToUse = (rimShaderProgram != 0) ? rimShaderProgram : defaultShaderProgram;

    glUseProgram(shaderToUse);

    if (shaderToUse == rimShaderProgram) {
        glUniform3fv(glGetUniformLocation(rimShaderProgram, "u_ViewPos"), 1, glm::value_ptr(cameraPos));
        glUniform3f(glGetUniformLocation(rimShaderProgram, "u_RimColor"), 0.3f, 0.7f, 1.0f); // Чуть мягче цвет
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_RimPower"), 4.5f);              // Сделать контур тоньше (было 3.0)
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_RimIntensity"), 1.0f);          // Уменьшить яркость (было 1.8)F
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_HitFlash"), getHitFlash());
        glUniform1f(glGetUniformLocation(rimShaderProgram, "u_Alpha"), 1.0f);
    }

    Node::render(shaderToUse, vpMatrix);

}