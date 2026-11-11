#include "../Game/EnemyNode.h"
#include <cstdlib>
#include <iostream>

void check(bool condition, const char* message) {
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    // Constructors and update paths need no GL context; only render uses GL.
    auto scene = std::make_shared<Node>();
    auto enemy = std::make_shared<EnemyNode>(0, 36);
    scene->addChild(enemy);
    for (int type = 0; type < 3; ++type) {
        enemy->spawn(type, 1.f, Vector3(6, 0, -6));
        scene->update(0.f);
        check(enemy->stats.name == std::array<const char*, 3>{"Slime", "Goblin", "Golem"}[type], "Spawn selects the right fighter");
        check(enemy->stats.hp == enemy->stats.maxHp && enemy->stats.alive(), "Spawn restores health");
        check(enemy->body->getScale().X == 1.f, "Alive model is visible");
        check(enemy->body->getPosition().X == 6.f, "Body keeps encounter position");
    }
    scene->setPosition(Vector3(2, 0, 0));
    scene->update(0.f);
    check(enemy->body->getWorldTransform()[3].x == 8.f, "Mob model participates in the scene hierarchy");
    enemy->stats.hp = 0;
    enemy->keepVisibleForFeedback(true);
    scene->update(0.f);
    check(enemy->body->getScale().X == 1.f, "Final hit remains visible during feedback");
    scene->update(1.f);
    check(enemy->dissolveProgress() == 0.f, "Dissolve waits for final-hit feedback");
    enemy->keepVisibleForFeedback(false);
    scene->update(0.f);
    check(enemy->body->getScale().X == 1.f, "Dissolve begins with the model visible");
    scene->update(0.45f);
    check(enemy->dissolveProgress() > 0.f && !enemy->dissolveFinished(), "Death progresses through dissolve");
    float pausedProgress = enemy->dissolveProgress();
    scene->update(0.f);
    check(enemy->dissolveProgress() == pausedProgress, "Pause freezes dissolve");
    scene->update(1.f);
    check(enemy->body->getScale().X == 0.f, "Defeated model disappears after feedback");
    check(enemy->dissolveFinished(), "Death completes before the reward transition");
    enemy->body->setHitFlash(1.f);
    enemy->body->setVisualOffset(Vector3(1, 0, 0));
    enemy->spawn(0, 2.f, Vector3());
    scene->update(0.f);
    check(enemy->stats.maxHp == 40 && enemy->stats.attack == 12, "Floor scaling is preserved");
    check(enemy->body->getHitFlash() == 0.f && enemy->body->getLocalTransform()[3].x == 0.f, "Respawn clears animation feedback");
    check(enemy->body->getScale().X == 1.f, "Respawn restores visibility");
    check(enemy->dissolveProgress() == 0.f && !enemy->dissolveFinished(), "Respawn resets dissolve");
    std::cout << "Enemy node lifecycle and scene hierarchy checks passed\n";
}
