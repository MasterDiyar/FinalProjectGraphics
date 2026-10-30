#include "Modules/Vector3.h"
#include "Modules/Node.h"
#include "Modules/MeshNode.h"
#include "Modules/ModelLoader.h" 
#include "Modules/Texture.h"  

#include <glad/glad.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include "Game/Battle.h"
#include "Game/Hud.h"
#include "Game/SpawnLayout.h"
#include <cmath>
#include <random>
#include <string>
#include <cctype>
#include <stdexcept>

GLuint shaderProgram = 0;
GLuint cubeVAO = 0;
int cubeIndexCount = 36;

std::shared_ptr<Node> rootScene;
GameState state = GameState::Explore;
Battle battle;
Hud hud;
Fighter playerStats{ "Hero", 40, 40, 9, 2, 10, 10 };

struct EnemyType {
    const char* name;
    int hp, atk, def;
    Vector3 color;
};

const EnemyType enemyTypes[] = {
    { "Slime",  20, 6, 1, Vector3(0.7f, 0.3f, 0.9f) },
    { "Goblin", 28, 8, 2, Vector3(0.9f, 0.5f, 0.1f) },
    { "Golem",  40, 7, 4, Vector3(0.5f, 0.5f, 0.55f) },
};

struct Enemy {
    Fighter stats;
    std::shared_ptr<MeshNode> body, hpBar;
    std::array<std::shared_ptr<Node>, 3> models;
};

std::vector<Enemy> enemies;
int currentEnemy = -1;
int floorNum = 1;
int kills = 0;
int metaAttackBonus = 0;

std::shared_ptr<MeshNode> player, floorNode, playerHpBar;
bool keys[256] = {};
int lastTime = 0;
int windowWidth = 800, windowHeight = 600;
constexpr float arenaLimit = 9.5f;

struct CombatMotion {
    std::shared_ptr<MeshNode> actor;
    Vector3 direction;
    float attackTime = 0.f, hitTime = 0.f, guardTime = 0.f;
    float reach = 0.4f;

    bool active() const { return attackTime > 0.f || hitTime > 0.f || guardTime > 0.f; }
    void strike(const std::shared_ptr<MeshNode>& source, const std::shared_ptr<MeshNode>& target, bool skill = false) {
        actor = source;
        Vector3 a = source->getPosition(), b = target->getPosition();
        float dx = b.X - a.X, dz = b.Z - a.Z;
        float length = std::sqrt(dx * dx + dz * dz);
        direction = length > 0.001f ? Vector3(dx / length, 0, dz / length) : Vector3(0, 0, -1);
        reach = skill ? 0.65f : 0.4f;
        attackTime = 0.36f;
    }
    void update(float dt) {
        if (!actor) return;
        attackTime = std::max(0.f, attackTime - dt);
        hitTime = std::max(0.f, hitTime - dt);
        guardTime = std::max(0.f, guardTime - dt);
        float lunge = std::sin(attackTime / 0.36f * 3.14159265f) * reach;
        float shake = hitTime > 0.f ? std::sin(hitTime * 75.f) * 0.07f : 0.f;
        actor->setVisualOffset(Vector3(direction.X * lunge + shake,
            -std::sin(guardTime / 0.36f * 3.14159265f) * 0.15f, direction.Z * lunge));
        actor->setHitFlash(hitTime / 0.3f);
    }
    void clear() {
        if (actor) { actor->setVisualOffset(Vector3()); actor->setHitFlash(0.f); }
        *this = CombatMotion{};
    }
};
CombatMotion playerMotion, enemyMotion;

// new shaders for texture support
const char* vertexShaderSrc =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 2) in vec2 aUV;\n"
"uniform mat4 u_MVP;\n"
"uniform mat4 u_Model;\n"
"out vec3 vWorld;\n"
"out vec2 vUV;\n"
"void main()\n"
"{\n"
"   vUV = aUV;\n"
"   vWorld = (u_Model * vec4(aPos, 1.0)).xyz;\n"
"   gl_Position = u_MVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSrc =
"#version 330 core\n"
"in vec2 vUV;\n"
"in vec3 vWorld;\n"
"out vec4 FragColor;\n"
"uniform vec3 u_Color;\n"
"uniform sampler2D u_Tex;\n"
"uniform bool u_UseTex;\n"
"uniform bool u_Lighting;\n"
"uniform float u_HitFlash;\n"
"void main() {\n"
"   vec3 c = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;\n"
"   c = mix(c, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));\n"
"   if (u_Lighting) {\n"
"       vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));\n"
"       c *= 0.45 + 0.55 * max(dot(n, normalize(vec3(-0.4, 1.0, 0.6))), 0.0);\n"
"   }\n"
"   FragColor = vec4(c, 1.0);\n"
"}\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compilation failed: ") + log);
    }
    return shader;
}

void initShaders() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[2048] = {};
        glGetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log);
        throw std::runtime_error(std::string("Shader linking failed: ") + log);
    }
}

void initCube() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
    };
    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        4, 5, 1,  1, 0, 4,
        7, 6, 2,  2, 3, 7,
        4, 7, 3,  3, 0, 4,
        5, 6, 2,  2, 1, 5
    };

    GLuint VBO, EBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void spawnEnemies() {
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> pickType(0, 2);
    const auto spawnPoints = makeSpawnLayout(rng);
    size_t spawnIndex = 0;
    float scale = 1.0f + 0.15f * (floorNum - 1);

    for (auto& e : enemies) {
        int typeIndex = pickType(rng);
        const EnemyType& t = enemyTypes[typeIndex];
        int hp = (int)(t.hp * scale);
        e.stats = Fighter{ t.name, hp, hp, (int)(t.atk * scale), t.def, 0, 0 };
        e.body->color = t.color;
        for (int model = 0; model < 3; ++model)
            e.models[model]->setScale(model == typeIndex ? Vector3(1, 1, 1) : Vector3());
        const auto& p = spawnPoints.at(spawnIndex++);
        e.body->setPosition(Vector3(p.x, 0, p.z));
    }
}

void resetRun() {
    playerMotion.clear();
    enemyMotion.clear();
    floorNum = 1;
    kills = 0;
    playerStats = Fighter{ "Hero", 40, 40, 9 + metaAttackBonus, 2, 10, 10 };
    spawnEnemies();
    player->setPosition(Vector3(0, 0, 0));
    state = GameState::Explore;
}

void finishUpgrade() {
    bool allDead = true;
    for (auto& e : enemies) if (e.stats.alive()) allDead = false;

    if (allDead) {
        floorNum++;
        spawnEnemies();
        player->setPosition(Vector3(0, 0, 0));
    }
    state = GameState::Explore;
}

std::shared_ptr<MeshNode> addBlock(const std::shared_ptr<Node>& parent,
    Vector3 position, Vector3 scale, Vector3 color) {
    auto block = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    block->setPosition(position);
    block->setScale(scale);
    block->color = color;
    parent->addChild(block);
    return block;
}

void decorateArena() {
    for (int x = 0; x < 10; ++x)
        for (int z = 0; z < 10; ++z) {
            bool path = x == 4 || x == 5 || z == 4 || z == 5;
            float shade = (x + z) % 2 ? 0.03f : 0.f;
            addBlock(rootScene, Vector3(-9.f + x * 2, -0.49f, -9.f + z * 2),
                Vector3(1.96f, 0.02f, 1.96f), path
                ? Vector3(0.55f + shade, 0.53f + shade, 0.43f + shade)
                : Vector3(0.25f + shade, 0.53f + shade, 0.30f + shade));
        }
    const Vector3 stone(0.36f, 0.42f, 0.49f);
    for (int side : {-1, 1}) {
        addBlock(rootScene, Vector3(side * 10.3f, -0.1f, 0), Vector3(0.6f, 0.8f, 21.2f), stone);
        addBlock(rootScene, Vector3(0, -0.1f, side * 10.3f), Vector3(20, 0.8f, 0.6f), stone);
        for (int corner : {-1, 1}) {
            addBlock(rootScene, Vector3(side * 10.3f, 0.5f, corner * 10.3f), Vector3(1, 2, 1), stone);
            addBlock(rootScene, Vector3(side * 10.3f, 1.6f, corner * 10.3f), Vector3(1.2f, 0.2f, 1.2f), Vector3(0.7f, 0.65f, 0.4f));
        }
    }
}

void buildEnemyModels(Enemy& enemy) {
    // Reuse the same encounter/animation anchor while switching child models.
    for (int type = 0; type < 3; ++type) {
        auto model = std::make_shared<Node>();
        enemy.models[type] = model;
        enemy.body->addChild(model);
        Vector3 color = enemyTypes[type].color;
        float eyeY = 0.05f, eyeZ = 0.52f;
        if (type == 0) {
            addBlock(model, Vector3(0, -0.18f, 0), Vector3(1.25f, 0.64f, 1.05f), color);
            addBlock(model, Vector3(0, 0.2f, -0.08f), Vector3(0.7f, 0.2f, 0.65f), color);
        }
        else if (type == 1) {
            addBlock(model, Vector3(0, -0.05f, 0), Vector3(0.65f, 0.8f, 0.5f), Vector3(0.38f, 0.25f, 0.14f));
            addBlock(model, Vector3(0, 0.62f, 0), Vector3(0.8f, 0.65f, 0.7f), color);
            for (float side : {-1.f, 1.f}) {
                auto ear = addBlock(model, Vector3(side * 0.55f, 0.73f, 0), Vector3(0.4f, 0.18f, 0.35f), color);
                ear->setRotation(Vector3(0, 0, side * 25.f));
                addBlock(model, Vector3(side * 0.48f, -0.02f, 0), Vector3(0.22f, 0.65f, 0.3f), color);
            }
            eyeY = 0.67f;
            eyeZ = 0.37f;
        }
        else {
            addBlock(model, Vector3(0, 0.1f, 0), Vector3(1.15f, 1.2f, 0.8f), color);
            addBlock(model, Vector3(0, 0.91f, 0), Vector3(0.7f, 0.5f, 0.65f), color);
            for (float side : {-1.f, 1.f}) {
                addBlock(model, Vector3(side * 0.79f, 0.05f, 0), Vector3(0.4f, 1.f, 0.65f), color);
                addBlock(model, Vector3(side * 0.35f, 0.78f, -0.15f), Vector3(0.23f, 0.3f, 0.4f), Vector3(0.32f, 0.35f, 0.4f));
            }
            eyeY = 0.95f;
            eyeZ = 0.35f;
        }
        for (float x : {-0.2f, 0.2f}) {
            addBlock(model, Vector3(x, eyeY, eyeZ), Vector3(0.2f, 0.22f, 0.04f), Vector3(0.95f, 0.95f, 0.86f));
            addBlock(model, Vector3(x, eyeY, eyeZ + 0.03f), Vector3(0.08f, 0.12f, 0.03f), Vector3(0.08f, 0.1f, 0.15f));
        }
        addBlock(model, Vector3(0, eyeY - 0.22f, eyeZ), Vector3(0.25f, 0.06f, 0.04f), Vector3(0.12f, 0.08f, 0.16f));
    }
}

void initScene() {
    rootScene = std::make_shared<Node>();

    floorNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    floorNode->setScale(Vector3(20, 0.2f, 20));
    floorNode->setPosition(Vector3(0, -0.6f, 0));
    floorNode->color = Vector3(0.3f, 0.7f, 0.3f);
    rootScene->addChild(floorNode);
    decorateArena();

	// use loadOBJ to load the hero model and create a MeshNode for the player
    Model heroModel = loadOBJ("assets/hero.obj");
    if (heroModel.vao != 0) {
        player = std::make_shared<MeshNode>(heroModel.vao, heroModel.indexCount);
        player->textureId = loadTexture("assets/hero.png");
        player->color = Vector3(1.0f, 1.0f, 1.0f); 
    }
    else {
        player = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        player->color = Vector3(0.2f, 0.5f, 1.0f);
        addBlock(player, Vector3(0, 0.83f, 0), Vector3(0.65f, 0.65f, 0.65f), Vector3(1.f, 0.79f, 0.53f));
        addBlock(player, Vector3(-0.65f, 0, 0), Vector3(0.25f, 0.8f, 0.4f), player->color);
        addBlock(player, Vector3(0.65f, 0, 0), Vector3(0.25f, 0.8f, 0.4f), player->color);
        for (float x : {-0.15f, 0.15f})
            addBlock(player, Vector3(x, 0.88f, 0.34f), Vector3(0.09f, 0.1f, 0.03f), Vector3(0.08f, 0.1f, 0.15f));
    }
    rootScene->addChild(player);

    playerHpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    playerHpBar->color = Vector3(0.1f, 0.9f, 0.1f);
    rootScene->addChild(playerHpBar);

    for (int i = 0; i < 3; i++) {
        Enemy e;
        e.body = std::make_shared<MeshNode>(cubeVAO, 0);
        buildEnemyModels(e);
        e.hpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        e.hpBar->color = Vector3(0.9f, 0.1f, 0.1f);
        rootScene->addChild(e.body);
        rootScene->addChild(e.hpBar);
        enemies.push_back(e);
    }
    spawnEnemies();
}

void chooseAction(unsigned char k) {
    if (state == GameState::Battle) {
        if (!battle.playerTurn || battle.finished || playerMotion.active() || enemyMotion.active()) return;
        if (k < '1' || k > '3') return;
        auto target = enemies[currentEnemy].body;
        int hpBefore = enemies[currentEnemy].stats.hp;
        if (k == '1') battle.playerAct(PlayerAction::Attack);
        if (k == '2') battle.playerAct(PlayerAction::Skill);
        if (k == '3') {
            battle.playerAct(PlayerAction::Defend);
            playerMotion.actor = player;
            playerMotion.guardTime = 0.36f;
        }
        if (enemies[currentEnemy].stats.hp < hpBefore) {
            playerMotion.strike(player, target, k == '2');
            enemyMotion.actor = target;
            enemyMotion.hitTime = 0.3f;
        }
    }
    else if (state == GameState::Upgrade) {
        if (k == '1')      playerStats.attack += 2;
        else if (k == '2') { playerStats.maxHp += 8; playerStats.hp = playerStats.maxHp; }
        else if (k == '3') { playerStats.maxMp += 4; playerStats.mp = playerStats.maxMp; }
        else return;
        finishUpgrade();
    }
    else if (state == GameState::Defeat && k == ' ') {
        resetRun();
    }
}

void keyDown(unsigned char k, int, int) {
    k = static_cast<unsigned char>(std::tolower(k));
    keys[k] = true;
    chooseAction(k);
}

bool actionEnabled(int index) {
    if (state == GameState::Upgrade || state == GameState::Defeat) return true;
    return state == GameState::Battle && battle.playerTurn && !battle.finished
        && !playerMotion.active() && !enemyMotion.active()
        && (index != 1 || playerStats.mp >= 3);
}

void mouseDown(int button, int pressed, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || pressed != GLUT_DOWN) return;
    float ux = x * 800.f / windowWidth, uy = y * 600.f / windowHeight;
    if (uy < 542 || uy >= 584) return;
    for (int i = 0; i < 3; ++i) {
        float left = 18.f + i * 260.f;
        if (ux < left || ux >= left + 244 || !actionEnabled(i)) continue;
        if (state == GameState::Defeat) {
            if (i == 0) chooseAction(' ');
        }
        else chooseAction(static_cast<unsigned char>('1' + i));
        break;
    }
}

void drawHud() {
    hud.begin(shaderProgram);
    hud.rect(0, 0, 800, 106, glm::vec3(0.055f, 0.075f, 0.12f));
    hud.text(18, 14, "MONSTER CLASH", glm::vec3(0.4f, 0.85f, 1.f));
    hud.text(430, 14, "FLOOR " + std::to_string(floorNum) + "  KILLS " + std::to_string(kills));
    hud.text(18, 42, "HP " + std::to_string(playerStats.hp) + "/" + std::to_string(playerStats.maxHp));
    hud.text(280, 42, "MP " + std::to_string(playerStats.mp) + "/" + std::to_string(playerStats.maxMp));
    hud.text(550, 42, "ATK " + std::to_string(playerStats.attack));
    hud.rect(18, 68, 235, 12, glm::vec3(0.16f));
    hud.rect(18, 68, 235.f * playerStats.hp / playerStats.maxHp, 12, glm::vec3(0.2f, 0.8f, 0.4f));
    hud.rect(280, 68, 235, 12, glm::vec3(0.16f));
    hud.rect(280, 68, 235.f * playerStats.mp / playerStats.maxMp, 12, glm::vec3(0.25f, 0.55f, 1.f));

    hud.rect(0, 452, 800, 148, glm::vec3(0.055f, 0.075f, 0.12f));
    std::string heading, message;
    std::array<std::string, 3> labels;
    if (state == GameState::Battle) {
        const Fighter& enemy = enemies[currentEnemy].stats;
        heading = enemy.name + "  HP " + std::to_string(enemy.hp) + "/" + std::to_string(enemy.maxHp);
        message = battle.log;
        labels = {"[1] ATTACK", "[2] SKILL - 3 MP", "[3] DEFEND"};
        std::string turn = battle.playerTurn ? "YOUR TURN" : "ENEMY TURN";
        if (playerMotion.active() || enemyMotion.active()) turn = "RESOLVING ACTION";
        hud.text(18, 88, turn, glm::vec3(1.f, 0.8f, 0.3f));
    }
    else if (state == GameState::Upgrade) {
        heading = "VICTORY! CHOOSE ONE UPGRADE";
        message = "+3 MP RESTORED. HEALTH AND MANA UPGRADES REFILL THEM.";
        labels = {"[1] +2 ATTACK", "[2] +8 HP / HEAL", "[3] +4 MP / REFILL"};
    }
    else if (state == GameState::Defeat) {
        heading = "YOU DIED - TRY ANOTHER RUN";
        message = "PERMANENT ATTACK BONUS: +" + std::to_string(metaAttackBonus) + " THIS SESSION";
        labels[0] = "SPACE / RESTART";
    }
    else {
        heading = "EXPLORE THE ARENA";
        message = "WASD TO MOVE. TOUCH AN ENEMY TO START A BATTLE.";
    }
    hud.text(18, 468, heading, glm::vec3(0.4f, 0.85f, 1.f));
    hud.text(18, 504, message.substr(0, 63));
    for (int i = 0; i < 3; ++i) {
        if (labels[i].empty()) continue;
        bool enabled = actionEnabled(i);
        hud.rect(18.f + i * 260, 542, 244, 42,
            enabled ? glm::vec3(0.16f, 0.32f, 0.48f) : glm::vec3(0.13f, 0.15f, 0.19f));
        hud.text(28.f + i * 260, 556, labels[i], enabled ? glm::vec3(0.96f) : glm::vec3(0.45f));
    }
    hud.end();
}

void keyUp(unsigned char k, int, int) {
    keys[static_cast<unsigned char>(std::tolower(k))] = false;
}

void reshape(int width, int height) {
    windowWidth = std::max(width, 1);
    windowHeight = std::max(height, 1);
    glViewport(0, 0, windowWidth, windowHeight);
}

void setBar(std::shared_ptr<MeshNode>& bar, Vector3 base, int hp, int maxHp) {
    float r = (float)hp / maxHp;
    bar->setScale(Vector3(std::max(r, 0.001f) * 1.5f, 0.15f, 0.15f));
    bar->setPosition(Vector3(base.X, base.Y + 1.6f, base.Z));
}

void updateGame(float dt) {
    playerMotion.update(dt);
    enemyMotion.update(dt);
    if (state == GameState::Explore) {
        Vector3 p = player->getPosition();
        float dx = float(keys['d']) - float(keys['a']);
        float dz = float(keys['s']) - float(keys['w']);
        float length = std::sqrt(dx * dx + dz * dz);
        if (length > 0.0f) {
            p.X += dx / length * 4.0f * dt;
            p.Z += dz / length * 4.0f * dt;
        }
        p.X = std::clamp(p.X, -arenaLimit, arenaLimit);
        p.Z = std::clamp(p.Z, -arenaLimit, arenaLimit);
        player->setPosition(p);

        for (size_t i = 0; i < enemies.size(); i++) {
            if (!enemies[i].stats.alive()) continue;
            Vector3 e = enemies[i].body->getPosition();
            float dx = p.X - e.X, dz = p.Z - e.Z;
            if (std::sqrt(dx * dx + dz * dz) < 1.3f) {
                currentEnemy = (int)i;
                playerMotion.clear();
                enemyMotion.clear();
                battle.start(&playerStats, &enemies[i].stats);
                state = GameState::Battle;
                break;
            }
        }
    }
    else if (state == GameState::Battle) {
        int hpBefore = playerStats.hp;
        battle.update(dt);
        if (playerStats.hp < hpBefore) {
            enemyMotion.strike(enemies[currentEnemy].body, player);
            playerMotion.actor = player;
            playerMotion.hitTime = 0.3f;
        }
        if (battle.finished && !playerMotion.active() && !enemyMotion.active()) {
            if (battle.playerWon) {
                kills++;
                playerStats.mp = std::min(playerStats.maxMp, playerStats.mp + 3);
                state = GameState::Upgrade;
            }
            else {
                metaAttackBonus++;
                state = GameState::Defeat;
            }
        }
    }

    setBar(playerHpBar, player->getPosition(), playerStats.hp, playerStats.maxHp);

    for (auto& e : enemies) {
        if (e.stats.alive() || (e.body == enemyMotion.actor && (enemyMotion.active() || playerMotion.active()))) {
            e.body->setScale(Vector3(1, 1, 1));
            setBar(e.hpBar, e.body->getPosition(), e.stats.hp, e.stats.maxHp);
        }
        else {
            e.body->setScale(Vector3(0, 0, 0));
            e.hpBar->setScale(Vector3(0, 0, 0));
        }
    }

    std::string t = "Floor " + std::to_string(floorNum) + " | Kills " + std::to_string(kills)
        + " | HP " + std::to_string(playerStats.hp) + "/" + std::to_string(playerStats.maxHp)
        + " MP " + std::to_string(playerStats.mp) + " ATK " + std::to_string(playerStats.attack);

    if (state == GameState::Battle)
        t += " | vs " + enemies[currentEnemy].stats.name + " | [1]Attack [2]Skill [3]Defend | " + battle.log;
    else if (state == GameState::Upgrade)
        t += " | LOOT: [1] +2 Attack  [2] +8 Max HP (heal)  [3] +4 Max MP";
    else if (state == GameState::Defeat)
        t += " | YOU DIED (permanent +1 ATK) - SPACE to restart";
    else
        t += " | WASD to move, touch an enemy";

    glutSetWindowTitle(t.c_str());
}

void display() {
    glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = std::clamp((now - lastTime) / 1000.0f, 0.0f, 0.1f);
    lastTime = now;

    updateGame(dt);

    Vector3 p = player->getPosition();
    glm::mat4 view = glm::lookAt(glm::vec3(p.X, p.Y + 7.0f, p.Z + 9.0f), glm::vec3(p.X, p.Y, p.Z), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), float(windowWidth) / windowHeight, 0.1f, 100.0f);

    rootScene->update(dt);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_Lighting"), GL_TRUE);
    rootScene->render(shaderProgram, proj * view);
    drawHud();
    glutSwapBuffers();
}

void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Scene Graph");

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    try {
        initShaders();
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    initCube();
    initScene();
    hud.init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc(mouseDown);
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
