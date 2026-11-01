#include "../Game/Battle.h"
#include <cstdlib>
#include <iostream>

void check(bool condition, const char* message) {
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    Fighter hero{"Hero", 100, 100, 9, 2, 10, 0};
    Fighter enemy{"Enemy", 1000, 1000, 8, 2, 0, 0};
    Battle battle;
    battle.start(&hero, &enemy);
    battle.playerAct(PlayerAction::Defend);
    check(hero.mp == 2 && hero.defending && !battle.playerTurn, "Defend must restore MP and spend a turn");
    battle.playerAct(PlayerAction::Defend);
    check(hero.mp == 2, "Repeated input during enemy turn must not restore MP");
    battle.update(1.f);
    check(battle.playerTurn && hero.hp < 100, "Enemy must respond to defense");
    int enemyHp = enemy.hp;
    battle.playerAct(PlayerAction::Skill);
    check(battle.playerTurn && hero.mp == 2 && enemy.hp == enemyHp && hero.defending,
        "Unaffordable skill must preserve turn, resources and defense");
    battle.playerAct(PlayerAction::Defend);
    check(hero.mp == 4, "Defense must allow saving enough MP for a skill");
    battle.update(1.f);
    battle.playerAct(PlayerAction::Skill);
    check(hero.mp == 1 && !hero.defending && enemy.hp < enemyHp && !battle.playerTurn,
        "Affordable skill must spend MP, deal damage and clear defense");

    for (int startingMp : {9, 10}) {
        hero.mp = startingMp;
        battle.start(&hero, &enemy);
        battle.playerAct(PlayerAction::Defend);
        check(hero.mp == 10, "Defense must cap MP at the maximum");
    }
    battle.finished = true;
    hero.mp = 0;
    battle.playerAct(PlayerAction::Defend);
    check(hero.mp == 0, "Finished battles must ignore actions");
    std::cout << "Battle defense/resource checks passed\n";
}
