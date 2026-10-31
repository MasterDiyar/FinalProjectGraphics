#pragma once
#include <algorithm>
#include <array>
#include <random>

struct SpawnPoint { float x, z; };

// Eight separated zones around a safe central start area. Jitter preserves a
// minimum 4.5-unit enemy separation without an unbounded rejection loop.
inline std::array<SpawnPoint, 8> makeSpawnLayout(std::mt19937& rng) {
    std::array<SpawnPoint, 8> points{{
        {-6, -6}, {0, -6}, {6, -6}, {-6, 0},
        {6, 0}, {-6, 6}, {0, 6}, {6, 6}
    }};
    std::shuffle(points.begin(), points.end(), rng);
    std::uniform_real_distribution<float> jitter(-0.75f, 0.75f);
    for (auto& point : points) {
        point.x += jitter(rng);
        point.z += jitter(rng);
    }
    return points;
}
