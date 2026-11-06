#include "ArenaNode.h"
#include "Texture.h"
#include <glad/glad.h>

ArenaNode::ArenaNode(unsigned int cubeVAO, int cubeIndexCount)
    : cubeVAO(cubeVAO), cubeIndexCount(cubeIndexCount) {

    // 1. Создаем пол с текстурой травы
    // Генерируем меш плоскости с UV-координатами прямо в коде
    float vertices[] = {
        // Позиции             // Нормали         // UV
        -0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2,  2, 3, 0 };

    unsigned int floorVAO, VBO, EBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(floorVAO);
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

    floorNode = std::make_shared<MeshNode>(floorVAO, 6);
    floorNode->textureId = loadTexture("assets/grass.png");
    floorNode->color = Vector3(1.0f, 1.0f, 1.0f);
    floorNode->setScale(Vector3(20.0f, 1.0f, 20.0f));
    floorNode->setPosition(Vector3(0.0f, -0.6f, 0.0f));
    addChild(floorNode);

    // 2. Строим декорации арены (стены, плитки дорожки)
    decorateArena();
}

std::shared_ptr<MeshNode> ArenaNode::addBlock(Vector3 position, Vector3 scale, Vector3 color) {
    auto block = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    block->setPosition(position);
    block->setScale(scale);
    block->color = color;
    addChild(block);
    arenaBlocks.push_back(block);
    return block;
}

void ArenaNode::decorateArena() {
    for (int x = 0; x < 10; ++x)
        for (int z = 0; z < 10; ++z) {
            bool path = x == 4 || x == 5 || z == 4 || z == 5;
            if (!path) continue; 

            float shade = (x + z) % 2 ? 0.03f : 0.f;
            addBlock(Vector3(-9.f + x * 2, -0.49f, -9.f + z * 2),
                Vector3(1.96f, 0.02f, 1.96f),
                Vector3(0.55f + shade, 0.53f + shade, 0.43f + shade));
        }

    const Vector3 stone(0.36f, 0.42f, 0.49f);
    float heightWall1 = 0.88f; 
    float heightWall2 = 0.72f; 

    int index = 0;
    for (int side : {-1, 1}) {
        float h = (index++ % 2 == 0) ? heightWall1 : heightWall2;

        addBlock(Vector3(side * 10.3f, -0.1f, 0), Vector3(0.6f, h, 21.2f), stone);
        addBlock(Vector3(0, -0.1f, side * 10.3f), Vector3(20, h == heightWall1 ? heightWall2 : heightWall1, 0.6f), stone);

        for (int corner : {-1, 1}) {
            addBlock(Vector3(side * 10.3f, 0.5f, corner * 10.3f), Vector3(1, 2, 1), stone);
            addBlock(Vector3(side * 10.3f, 1.6f, corner * 10.3f), Vector3(1.2f, 0.2f, 1.2f), Vector3(0.7f, 0.65f, 0.4f));
        }
    }
}