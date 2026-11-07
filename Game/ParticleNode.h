#pragma once
#include "../Modules/MeshNode.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

// A pool of small cubes for short bursts such as hit sparks. Particles are
// children of this node, so they render with the rest of the scene and freeze
// when the scene is updated with dt = 0 (pause).
class ParticleNode : public Node {
public:
    ParticleNode(unsigned int cubeVao, int cubeIndexCount, int poolSize = 64) {
        particles.resize(poolSize);
        for (auto& p : particles) {
            p.mesh = std::make_shared<MeshNode>(cubeVao, cubeIndexCount);
            p.mesh->setScale(Vector3()); // hidden until used
            addChild(p.mesh);
        }
    }

    // Launches up to `count` free particles outwards from `origin`.
    void burst(Vector3 origin, Vector3 color, int count = 14, float speed = 3.f) {
        std::uniform_real_distribution<float> unit(0.f, 1.f);
        for (auto& p : particles) {
            if (count <= 0) break;
            if (p.life > 0.f) continue;
            --count;
            const float angle = unit(rng) * 6.2831853f;
            const float spread = (0.3f + 0.7f * unit(rng)) * speed;
            p.x = origin.X; p.y = origin.Y; p.z = origin.Z;
            p.vx = std::cos(angle) * spread;
            p.vz = std::sin(angle) * spread;
            p.vy = 1.5f + unit(rng) * 2.5f;
            p.maxLife = p.life = 0.35f + unit(rng) * 0.35f;
            p.size = 0.1f + unit(rng) * 0.12f;
            const float shade = 0.8f + 0.2f * unit(rng);
            p.mesh->color = Vector3(color.X * shade, color.Y * shade, color.Z * shade);
            p.mesh->setPosition(origin);
            p.mesh->setScale(Vector3(p.size, p.size, p.size));
        }
    }

    int aliveCount() const {
        return static_cast<int>(std::count_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life > 0.f; }));
    }

    void _Process(float dt) override {
        for (auto& p : particles) {
            if (p.life <= 0.f) continue;
            p.life -= dt;
            if (p.life <= 0.f) { p.mesh->setScale(Vector3()); continue; }
            p.vy -= 9.f * dt;
            p.x += p.vx * dt; p.y += p.vy * dt; p.z += p.vz * dt;
            if (p.y < groundY) { p.y = groundY; p.vy = 0.f; p.vx *= 0.8f; p.vz *= 0.8f; }
            const float s = p.size * (p.life / p.maxLife); // shrink as it fades
            p.mesh->setPosition(Vector3(p.x, p.y, p.z));
            p.mesh->setScale(Vector3(s, s, s));
        }
    }

private:
    struct Particle {
        std::shared_ptr<MeshNode> mesh;
        float x = 0, y = 0, z = 0, vx = 0, vy = 0, vz = 0;
        float life = 0, maxLife = 1, size = 0.15f;
    };
    static constexpr float groundY = -0.55f; // just above the arena floor
    std::vector<Particle> particles;
    std::mt19937 rng{ std::random_device{}() };
};
