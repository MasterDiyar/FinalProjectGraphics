#include "../Game/SpawnLayout.h"
#include <cmath>
#include <iostream>

int main() {
    for (unsigned seed = 0; seed < 10000; ++seed) {
        std::mt19937 rng(seed);
        const auto points = makeSpawnLayout(rng);
        for (size_t i = 0; i < points.size(); ++i) {
            auto p = points[i];
            if (std::abs(p.x) > 6.75f || std::abs(p.z) > 6.75f
                || p.x * p.x + p.z * p.z < 5.25f * 5.25f) return 1;
            for (size_t j = 0; j < i; ++j) {
                float dx = p.x - points[j].x, dz = p.z - points[j].z;
                if (dx * dx + dz * dz < 4.5f * 4.5f) return 2;
            }
        }
    }
    std::cout << "Spawn layout: 10000 seeds passed\n";
}
