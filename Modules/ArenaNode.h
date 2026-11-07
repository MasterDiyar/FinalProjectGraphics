#pragma once
#include "Node.h"
#include "MeshNode.h"
#include <memory>
#include <vector>

class ArenaNode : public Node {
private:
    std::shared_ptr<MeshNode> floorNode;
    std::vector<std::shared_ptr<MeshNode>> arenaBlocks;
    unsigned int cubeVAO;
    int cubeIndexCount;

    void initFloorMesh();
    void decorateArena();

public:
    ArenaNode(unsigned int cubeVAO, int cubeIndexCount);
    std::shared_ptr<MeshNode> addBlock(Vector3 position, Vector3 scale, Vector3 color, unsigned int textureId = 0);
};