#include "../Game/HudState.h"
#include <cstdlib>
#include <iostream>
#include <utility>

void check(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    HudState hud;
    check(hud.actionAt(30, 556, 800, 600) == 0, "Exploration has no action buttons");
    hud.state = GameState::Battle;
    check(hud.actionAt(30, 556, 800, 600) == '1', "Attack hit test");
    check(hud.actionAt(600, 1112, 1600, 1200) == '2', "Scaled skill hit test");
    check(hud.actionAt(550, 556, 800, 600) == '3', "Defend hit test");
    check(hud.buttonAt(30, 556, 800, 600) == 0 && hud.buttonAt(550, 556, 800, 600) == 2, "Hover finds buttons");
    check(hud.buttonAt(262, 556, 800, 600) == -1 && hud.buttonAt(-1, -1, 800, 600) == -1, "Hover ignores gaps and no mouse");
    for (auto point : {std::pair{17, 556}, {262, 556}, {30, 541}, {30, 584}})
        check(hud.actionAt(point.first, point.second, 800, 600) == 0, "Button borders/gaps must not activate");
    check(hud.actionAt(30, 556, 0, 600) == 0, "Zero viewport must not divide by zero");
    hud.player.mp = 2;
    check(!hud.actionEnabled(1) && hud.actionEnabled(0), "Only unaffordable skill is disabled");
    check(hud.buttonAt(300, 556, 800, 600) == -1, "Disabled buttons are not hovered");
    hud.resolving = true;
    check(hud.actionAt(30, 556, 800, 600) == 0, "Feedback blocks clicks");
    hud.resolving = false;
    hud.playerTurn = false;
    check(!hud.actionEnabled(0), "Enemy turn blocks clicks");
    hud.playerTurn = true;
    hud.finished = true;
    check(!hud.actionEnabled(0), "Finished battle blocks clicks");
    hud.state = GameState::Upgrade;
    check(hud.actionAt(550, 556, 800, 600) == '3', "Upgrade choice remains available");
    hud.paused = true;
    check(!hud.actionEnabled(0), "Pause blocks upgrades too");
    hud.paused = false;
    hud.state = GameState::Defeat;
    check(hud.actionAt(30, 556, 800, 600) == ' ', "Restart maps to Space");
    check(!hud.actionEnabled(1) && !hud.actionEnabled(2), "Only restart exists in defeat");
    check(!hud.actionEnabled(-1) && !hud.actionEnabled(3), "Invalid button indexes are rejected");
    std::cout << "HUD state and scaled input checks passed\n";
}
