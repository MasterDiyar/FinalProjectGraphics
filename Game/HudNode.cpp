#include "HudNode.h"

namespace {
const glm::vec3 kPanel(0.055f, 0.075f, 0.12f);
const glm::vec3 kBorder(0.16f, 0.24f, 0.36f);
const glm::vec3 kAccent(0.4f, 0.85f, 1.f);
const glm::vec3 kGold(1.f, 0.8f, 0.3f);

float fraction(int value, int maxValue) {
    return maxValue > 0 ? std::clamp(static_cast<float>(value) / maxValue, 0.f, 1.f) : 0.f;
}

// Green while healthy, yellow when hurt, red when low.
glm::vec3 hpColor(float ratio) {
    if (ratio > 0.5f) return glm::vec3(0.2f, 0.8f, 0.4f);
    if (ratio > 0.25f) return glm::vec3(0.95f, 0.75f, 0.2f);
    return glm::vec3(0.9f, 0.25f, 0.25f);
}

struct Label { std::string key, text; };

// A 244 x 42 button at the shared hit-test position, with a key badge on the left.
void drawButton(Hud& hud, float x, const Label& label, bool enabled, bool hovered) {
    const float y = 542, w = 244, h = 42;
    const glm::vec3 fill = !enabled ? glm::vec3(0.09f, 0.11f, 0.15f)
        : hovered ? glm::vec3(0.2f, 0.4f, 0.6f) : glm::vec3(0.14f, 0.28f, 0.44f);
    const glm::vec3 border = !enabled ? glm::vec3(0.18f, 0.2f, 0.25f)
        : hovered ? kGold : kAccent * 0.8f;
    hud.panel(x, y, w, h, fill, border);
    const float badge = std::max(28.f, Hud::textWidth(label.key) + 12.f);
    hud.panel(x + 7, y + 7, badge, 28, glm::vec3(0.04f, 0.06f, 0.1f), border);
    hud.text(x + 7 + (badge - Hud::textWidth(label.key)) / 2, y + 14, label.key,
        enabled ? kGold : glm::vec3(0.4f));
    hud.text(x + 7 + badge + 10, y + 14, label.text, enabled ? glm::vec3(0.96f) : glm::vec3(0.45f));
}
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
    hud.shadowText(18, 12, "MONSTER CLASH", kAccent, 3.f);
    hud.text(430, 14, "FLOOR " + std::to_string(floorNum) + "  KILLS " + std::to_string(kills));
    hud.text(18, 42, "HP " + std::to_string(playerStats.hp) + "/" + std::to_string(playerStats.maxHp));
    hud.text(280, 42, "MP " + std::to_string(playerStats.mp) + "/" + std::to_string(playerStats.maxMp));
    hud.text(550, 42, "ATK " + std::to_string(playerStats.attack));
    hud.text(550, 88, "P - PAUSE", glm::vec3(0.65f, 0.75f, 0.85f));
    hud.text(280, 88, "ENEMIES " + std::to_string(remaining) + "/" + std::to_string(model.total));
    const float hpRatio = fraction(playerStats.hp, playerStats.maxHp);
    hud.bar(18, 66, 235, 16, hpRatio, hpColor(hpRatio));
    hud.bar(280, 66, 235, 16, fraction(playerStats.mp, playerStats.maxMp), glm::vec3(0.25f, 0.55f, 1.f));

    if (state == GameState::Defeat) {
        hud.rect(0, 114, 800, 338, glm::vec3(0.1f, 0.f, 0.f), 0.55f);
        hud.centerText(240, "YOU DIED", glm::vec3(0.95f, 0.3f, 0.3f), 7.f);
        hud.centerText(312, "TRY ANOTHER RUN", glm::vec3(0.94f));
    }
    if (state == GameState::Upgrade) {
        hud.rect(0, 225, 800, 100, glm::vec3(0.f), 0.45f);
        hud.centerText(252, remaining == 0 ? "FLOOR CLEAR!" : "VICTORY!", kGold, 6.f);
    }
    hud.panel(0, 452, 800, 148, kPanel, kBorder);
    hud.rect(0, 452, 800, 2, kAccent * 0.6f);
    std::string heading, message;
    std::array<Label, 3> labels;
    if (state == GameState::Battle) {
        const Fighter& enemy = model.enemy;
        heading = enemy.name + "  HP " + std::to_string(enemy.hp) + "/" + std::to_string(enemy.maxHp);
        message = model.log;
        labels = {Label{"1", "ATTACK"}, Label{"2", "SKILL - 3 MP"}, Label{"3", "DEFEND +2 MP"}};
        hud.bar(18, 486, 300, 12, fraction(enemy.hp, enemy.maxHp), glm::vec3(0.85f, 0.25f, 0.25f));
        std::string turn = model.playerTurn ? "YOUR TURN" : "ENEMY TURN";
        if (model.resolving) turn = "RESOLVING ACTION";
        const glm::vec3 tone = model.resolving ? kGold
            : model.playerTurn ? glm::vec3(0.3f, 0.9f, 0.5f) : glm::vec3(0.95f, 0.4f, 0.35f);
        hud.panel(18, 84, Hud::textWidth(turn) + 16, 20, tone * 0.25f, tone);
        hud.text(26, 87, turn, tone);
    }
    else if (state == GameState::Upgrade) {
        heading = "CHOOSE ONE UPGRADE";
        message = "+3 MP RESTORED. HEALTH AND MANA UPGRADES REFILL THEM.";
        labels = {Label{"1", "+2 ATTACK"}, Label{"2", "+8 HP / HEAL"}, Label{"3", "+4 MP / REFILL"}};
        if (remaining == 0) {
            heading = "CHOOSE AN UPGRADE";
            message = "YOUR CHOICE STARTS THE NEXT, HARDER FLOOR.";
        }
    }
    else if (state == GameState::Defeat) {
        heading = "RUN OVER";
        message = "PERMANENT ATTACK BONUS: +" + std::to_string(metaAttackBonus) + " THIS SESSION";
        labels[0] = Label{"SPACE", "RESTART"};
    }
    else {
        heading = "EXPLORE THE ARENA";
        message = "WASD TO MOVE, Q/E TO ROTATE CAMERA. TOUCH AN ENEMY TO FIGHT.";
        hud.rect(18, 550, 764, 10, glm::vec3(0.16f));
        if (model.total != 0)
            hud.rect(18, 550, 764.f * static_cast<float>(cleared) / model.total, 10,
                glm::vec3(0.2f, 0.8f, 0.4f));
        hud.text(18, 575, "FLOOR PROGRESS " + std::to_string(cleared) + "/" + std::to_string(model.total));
    }
    hud.shadowText(18, 468, heading, kAccent);
    hud.text(18, 504, message.substr(0, 63));
    for (int i = 0; i < 3; ++i) {
        if (labels[i].text.empty()) continue;
        drawButton(hud, 18.f + i * 260, labels[i], model.actionEnabled(i), i == model.hover);
    }
    if (paused) {
        hud.rect(0, 0, 800, 600, glm::vec3(0.f), 0.55f);
        hud.panel(164, 200, 472, 160, glm::vec3(0.035f, 0.05f, 0.09f), kGold);
        hud.centerText(222, "PAUSED", kGold, 4.f);
        hud.centerText(286, "PRESS P TO CONTINUE", glm::vec3(0.94f));
        hud.centerText(316, "YOUR RUN IS ON HOLD", glm::vec3(0.65f, 0.75f, 0.85f));
    }
    hud.end();
}
