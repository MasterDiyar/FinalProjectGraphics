#include "ArenaNode.h"
#include "Texture.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
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

static GLuint compileShader(GLenum type, const std::string& source) {
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

void ArenaNode::initTexturedCube() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    unsigned int indices[] = {
        0,1,2, 2,3,0,   4,5,6, 6,7,4,   8,9,10, 10,11,8,
        12,13,14, 14,15,12, 16,17,18, 18,19,16, 20,21,22, 22,23,20
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &texturedCubeVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(texturedCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

ArenaNode::ArenaNode(unsigned int cubeVAO, int cubeIndexCount)
    : cubeVAO(cubeVAO), cubeIndexCount(cubeIndexCount) {
    initTexturedCube();

    float floorVertices[] = {
        -0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };
    unsigned int floorIndices[] = { 0, 1, 2,  2, 3, 0 };

    unsigned int floorVAO, fVBO, fEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &fVBO);
    glGenBuffers(1, &fEBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    try {
        std::string roadVertCode = loadShaderFile("shaders/player_sierpinski.vert");
        std::string roadFragCode = loadShaderFile("shaders/player_sierpinski.frag");

        GLuint rvs = compileShader(GL_VERTEX_SHADER, roadVertCode);
        GLuint rfs = compileShader(GL_FRAGMENT_SHADER, roadFragCode);
        noizeShaderProgram = glCreateProgram();
        glAttachShader(noizeShaderProgram, rvs);
        glAttachShader(noizeShaderProgram, rfs);
        glLinkProgram(noizeShaderProgram);
        glDeleteShader(rvs);
        glDeleteShader(rfs);
    }
    catch (const std::exception& e) {
        std::cerr << "Road shader loading error: " << e.what() << std::endl;
    }

    floorNode = std::make_shared<MeshNode>(floorVAO, 6);
    floorNode->textureId = loadTexture("assets/grass.png");
    floorNode->color = Vector3(1.0f, 1.0f, 1.0f);
    floorNode->setScale(Vector3(20.0f, 1.0f, 20.0f));
    floorNode->setPosition(Vector3(0.0f, -0.6f, 0.0f));
    addChild(floorNode);

    decorateArena();
}

std::shared_ptr<MeshNode> ArenaNode::addBlock(Vector3 position, Vector3 scale, Vector3 color, unsigned int textureId, bool addToSceneTree) {
    auto block = std::make_shared<MeshNode>(texturedCubeVAO, texturedCubeIndexCount);
    block->setPosition(position);
    block->setScale(scale);
    block->color = color;
    block->textureId = textureId;
    if (addToSceneTree) 
        addChild(block);
    arenaBlocks.push_back(block);
    return block;
}

void ArenaNode::decorateArena() {
    unsigned int roadTex = loadTexture("assets/road.png");
    roadTextureId = roadTex;
    roadBlocks.clear();

    for (int x = 0; x < 10; ++x) {
        for (int z = 0; z < 10; ++z) {
            bool path = x == 4 || x == 5 || z == 4 || z == 5;
            if (!path) continue;

            auto roadBlock = addBlock(
                Vector3(-9.f + x * 2, -0.49f, -9.f + z * 2),
                Vector3(1.96f, 0.02f, 1.96f),
                Vector3(1.0f, 1.0f, 1.0f),
                roadTex
            );
            roadBlocks.push_back(roadBlock);
        }
    }

    unsigned int wallTex = loadTexture("assets/wall.png");

    float heightWall1 = 0.95f;
    float heightWall2 = 0.82f;

    int index = 0;
    for (int side : {-1, 1}) {
        float h = (index++ % 2 == 0) ? heightWall1 : heightWall2;

        addBlock(Vector3(side * 10.3f, -0.35f, 0), Vector3(0.6f, h, 21.2f), Vector3(1.0f, 1.0f, 1.0f), wallTex);
        addBlock(Vector3(0, -0.35f, side * 10.3f), Vector3(20, h == heightWall1 ? heightWall2 : heightWall1, 0.6f), Vector3(1.0f, 1.0f, 1.0f), wallTex);

        for (int corner : {-1, 1}) {
            addBlock(Vector3(side * 10.3f, 0.5f, corner * 10.3f), Vector3(1, 2, 1), Vector3(1.0f, 1.0f, 1.0f), wallTex);
            addBlock(Vector3(side * 10.3f, 1.6f, corner * 10.3f), Vector3(1.2f, 0.2f, 1.2f), Vector3(0.7f, 0.65f, 0.4f), 0);
        }
    }
}
void ArenaNode::render(GLuint shaderProgram, const glm::mat4& viewProjMatrix) {
    glUseProgram(shaderProgram);
    if (floorNode) {
        floorNode->render(shaderProgram, viewProjMatrix);
    }
    for (const auto& block : arenaBlocks) {
        if (!block) continue;
        if (std::find(roadBlocks.begin(), roadBlocks.end(), block) == roadBlocks.end()) {
            block->render(shaderProgram, viewProjMatrix);
        }
    }

    // 2. Блоки дороги — через noizeShaderProgram
    if (noizeShaderProgram != 0) {
        glUseProgram(noizeShaderProgram);
        glUniform1i(glGetUniformLocation(noizeShaderProgram, "u_Tex"), 0);
        glUniform1i(glGetUniformLocation(noizeShaderProgram, "u_Lighting"), 1);
        glUniform3f(glGetUniformLocation(noizeShaderProgram, "u_SunDir"), 0.5f, 1.0f, 0.3f);
        glUniform1f(glGetUniformLocation(noizeShaderProgram, "u_Alpha"), 1.0f);
        glUniform1i(glGetUniformLocation(noizeShaderProgram, "u_UseNoise"), 1);

        for (const auto& block : roadBlocks) {
            if (block) {
                block->render(noizeShaderProgram, viewProjMatrix);
            }
        }
    }

    // Возвращаем активный шейдер обратно на основной
    glUseProgram(shaderProgram);
}