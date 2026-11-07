#pragma once
#include "Battle.h"
#include <cstddef>

// A value snapshot: HUD rendering never owns or mutates combat state.
struct HudState {
    GameState state = GameState::Explore;
    Fighter player;
    Fighter enemy;
    int floor = 1, kills = 0, metaAttackBonus = 0;
    size_t remaining = 0, total = 0;
    bool paused = false, resolving = false, playerTurn = true, finished = false;
    int hover = -1; // index of the enabled button under the mouse, or -1
    std::string log;

    bool actionEnabled(int index) const {
        if (index < 0 || index > 2 || paused) return false;
        if (state == GameState::Defeat) return index == 0;
        if (state == GameState::Upgrade) return true;
        return state == GameState::Battle && playerTurn && !finished
            && !resolving && (index != 1 || player.mp >= 3);
    }

    // Index (0-2) of the enabled button at a window position, or -1.
    int buttonAt(int x, int y, int width, int height) const {
        if (width <= 0 || height <= 0) return -1;
        float ux = x * 800.f / width, uy = y * 600.f / height;
        if (uy < 542 || uy >= 584) return -1;
        for (int i = 0; i < 3; ++i) {
            float left = 18.f + i * 260.f;
            if (ux >= left && ux < left + 244 && actionEnabled(i)) return i;
        }
        return -1;
    }

    unsigned char actionAt(int x, int y, int width, int height) const {
        int i = buttonAt(x, y, width, height);
        if (i < 0) return 0;
        return state == GameState::Defeat ? ' ' : static_cast<unsigned char>('1' + i);
    }
};
