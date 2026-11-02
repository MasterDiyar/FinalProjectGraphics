#pragma once
#include "../Modules/Node.h"
#include "Hud.h"
#include "HudState.h"

// Add last to the root scene so the screen-space pass follows world geometry.
class HudNode : public Node {
public:
    void init() { hud.init(); }
    void setState(const HudState& value) { model = value; }
    unsigned char actionAt(int x, int y, int width, int height) const {
        return model.actionAt(x, y, width, height);
    }
    void render(GLuint shaderProgram, const glm::mat4& viewProjection) override;

private:
    Hud hud;
    HudState model;
};
