#include "HudNode.h"

namespace {
const glm::vec3 kPanel(0.055f, 0.075f, 0.12f);
const glm::vec3 kBorder(0.16f, 0.24f, 0.36f);
const glm::vec3 kAccent(0.4f, 0.85f, 1.f);
}

void HudNode::render(GLuint shaderProgram, const glm::mat4&) {
    const auto state = model.state;
    const auto& playerStats = model.player;
    const int floorNum = model.floor, kills = model.kills, metaAttackBonus = model.metaAttackBonus;
    const bool paused = model.paused;
    const size_t remaining = model.remaining;
    const size_t cleared = model.total - remaining;
    hud.begin(shaderProgram);
    hud.panel(0, 0, 800, 114, kPanel, kBorder);
    hud.rect(0, 112, 800, 2, kAccent * 0.6f);
    hud.text(18, 14, "MONSTER CLASH", glm::vec3(0.4f, 0.85f, 1.f));
    hud.text(430, 14, "FLOOR " + std::to_string(floorNum) + "  KILLS " + std::to_string(kills));
    hud.text(18, 42, "HP " + std::to_string(playerStats.hp) + "/" + std::to_string(playerStats.maxHp));
    hud.text(280, 42, "MP " + std::to_string(playerStats.mp) + "/" + std::to_string(playerStats.maxMp));
    hud.text(550, 42, "ATK " + std::to_string(playerStats.attack));
    hud.text(550, 88, "P - PAUSE", glm::vec3(0.65f, 0.75f, 0.85f));
    hud.text(280, 88, "ENEMIES " + std::to_string(remaining) + "/" + std::to_string(model.total));
    hud.rect(18, 68, 235, 12, glm::vec3(0.16f));
    hud.rect(18, 68, 235.f * playerStats.hp / playerStats.maxHp, 12, glm::vec3(0.2f, 0.8f, 0.4f));
    hud.rect(280, 68, 235, 12, glm::vec3(0.16f));
    hud.rect(280, 68, 235.f * playerStats.mp / playerStats.maxMp, 12, glm::vec3(0.25f, 0.55f, 1.f));

    hud.panel(0, 452, 800, 148, kPanel, kBorder);
    hud.rect(0, 452, 800, 2, kAccent * 0.6f);
    std::string heading, message;
    std::array<std::string, 3> labels;
    if (state == GameState::Battle) {
        const Fighter& enemy = model.enemy;
        heading = enemy.name + "  HP " + std::to_string(enemy.hp) + "/" + std::to_string(enemy.maxHp);
        message = model.log;
        labels = {"[1] ATTACK", "[2] SKILL - 3 MP", "[3] DEFEND +2 MP"};
        std::string turn = model.playerTurn ? "YOUR TURN" : "ENEMY TURN";
        if (model.resolving) turn = "RESOLVING ACTION";
        hud.text(18, 88, turn, glm::vec3(1.f, 0.8f, 0.3f));
    }
    else if (state == GameState::Upgrade) {
        heading = "VICTORY! CHOOSE ONE UPGRADE";
        message = "+3 MP RESTORED. HEALTH AND MANA UPGRADES REFILL THEM.";
        labels = {"[1] +2 ATTACK", "[2] +8 HP / HEAL", "[3] +4 MP / REFILL"};
        if (remaining == 0) {
            heading = "FLOOR CLEAR! CHOOSE AN UPGRADE";
            message = "YOUR CHOICE STARTS THE NEXT, HARDER FLOOR.";
        }
    }
    else if (state == GameState::Defeat) {
        heading = "YOU DIED - TRY ANOTHER RUN";
        message = "PERMANENT ATTACK BONUS: +" + std::to_string(metaAttackBonus) + " THIS SESSION";
        labels[0] = "SPACE / RESTART";
    }
    else {
        heading = "EXPLORE THE ARENA";
        message = "WASD TO MOVE. TOUCH AN ENEMY TO START A BATTLE.";
        hud.rect(18, 550, 764, 10, glm::vec3(0.16f));
        if (model.total != 0)
            hud.rect(18, 550, 764.f * static_cast<float>(cleared) / model.total, 10,
                glm::vec3(0.2f, 0.8f, 0.4f));
        hud.text(18, 575, "FLOOR PROGRESS " + std::to_string(cleared) + "/" + std::to_string(model.total));
    }
    hud.text(18, 468, heading, glm::vec3(0.4f, 0.85f, 1.f));
    hud.text(18, 504, message.substr(0, 63));
    for (int i = 0; i < 3; ++i) {
        if (labels[i].empty()) continue;
        bool enabled = model.actionEnabled(i);
        hud.rect(18.f + i * 260, 542, 244, 42,
            enabled ? glm::vec3(0.16f, 0.32f, 0.48f) : glm::vec3(0.13f, 0.15f, 0.19f));
        hud.text(28.f + i * 260, 556, labels[i], enabled ? glm::vec3(0.96f) : glm::vec3(0.45f));
    }
    if (paused) {
        hud.rect(164, 214, 472, 126, glm::vec3(0.035f, 0.05f, 0.09f));
        hud.text(364, 238, "PAUSED", glm::vec3(1.f, 0.8f, 0.3f));
        hud.text(226, 276, "PRESS P TO CONTINUE");
        hud.text(226, 306, "YOUR RUN IS ON HOLD", glm::vec3(0.65f, 0.75f, 0.85f));
    }
    hud.end();
}
