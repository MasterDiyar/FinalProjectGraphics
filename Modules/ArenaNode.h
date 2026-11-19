#pragma once
#include "Node.h"
#include "MeshNode.h"
#include <memory>
#include <vector>
#include "ModelLoader.h"

class ArenaNode : public Node {
private:
    std::shared_ptr<MeshNode> floorNode;
    std::vector<std::shared_ptr<MeshNode>> arenaBlocks;
    unsigned int cubeVAO;
    int cubeIndexCount;

    unsigned int texturedCubeVAO = 0;
    int texturedCubeIndexCount = 36;
    void initTexturedCube();

    unsigned int roadTextureId = 0;
    std::vector<std::shared_ptr<MeshNode>> roadBlocks;
    GLuint roadNoiseShader = 0;
    GLuint noizeShaderProgram = 0;

    void initFloorMesh();
    void decorateArena();

public:
    ArenaNode(unsigned int cubeVAO, int cubeIndexCount);
    void render(GLuint shaderProgram, const glm::mat4& viewProjMatrix) override;
    std::shared_ptr<MeshNode> addBlock(Vector3 position, Vector3 scale, Vector3 color, unsigned int textureId = 0, bool addToSceneTree = true);
};