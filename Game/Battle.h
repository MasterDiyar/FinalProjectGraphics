#pragma once
#include <string>
#include <random>
#include <algorithm>

enum class GameState { Explore, Battle, Victory, Defeat, Upgrade };
enum class PlayerAction { Attack, Skill, Defend };

struct Fighter {
    std::string name;
    int maxHp = 30, hp = 30;
    int attack = 8, defense = 2;
    int maxMp = 10, mp = 10;
    bool defending = false;
    bool alive() const { return hp > 0; }
};

class Battle {
public:
    Fighter* player = nullptr;
    Fighter* enemy = nullptr;
    std::string log = "";
    bool playerTurn = true;
    bool finished = false;
    bool playerWon = false;

    void start(Fighter* p, Fighter* e) {
        player = p; enemy = e;
        playerTurn = true; finished = false; playerWon = false;
        enemyTimer = 0.f;
        log = "A wild " + e->name + " appears!";
    }
 
    // called when the player chooses an action 1/2/3
    void playerAct(PlayerAction a) {
        if (!playerTurn || finished) return;
        player->defending = false;

        if (a == PlayerAction::Attack) {
            int d = calcDamage(player->attack, enemy->defense, enemy->defending);
            enemy->hp -= d;
            log = "You hit for " + std::to_string(d);
        } else if (a == PlayerAction::Skill) {
            if (player->mp < 3) { log = "Not enough MP!"; return; }
            player->mp -= 3;
            int d = calcDamage(player->attack * 2, enemy->defense, enemy->defending);
            enemy->hp -= d;
            log = "Skill hits for " + std::to_string(d);
        } else {
            player->defending = true;
            log = "You brace for impact";
        }

        if (!enemy->alive()) { enemy->hp = 0; finished = true; playerWon = true; log += " - Victory!"; return; }
        playerTurn = false;
        enemyTimer = 0.9f; // pause before enemy attacks
    }

    void update(float dt) {
        if (finished || playerTurn) return;
        enemyTimer -= dt;
        if (enemyTimer > 0.f) return;

        enemy->defending = false;
        int d = calcDamage(enemy->attack, player->defense, player->defending);
        player->hp -= d;
        log = enemy->name + " hits you for " + std::to_string(d);

        if (!player->alive()) { player->hp = 0; finished = true; playerWon = false; log += " - You died"; return; }
        playerTurn = true;
    }

private:
    float enemyTimer = 0.f;
    std::mt19937 rng{ std::random_device{}() };

    int calcDamage(int atk, int def, bool defending) {
        std::uniform_int_distribution<int> var(-2, 2);
        int d = std::max(1, atk - def / 2 + var(rng));
        if (defending) d = std::max(1, d / 2);
        return d;
    }
};