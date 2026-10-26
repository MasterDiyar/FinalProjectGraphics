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

// new shaders for texture support
const char* vertexShaderSrc =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 2) in vec2 aUV;\n"
"uniform mat4 u_MVP;\n"
"out vec2 vUV;\n"
"void main()\n"
"{\n"
"   vUV = aUV;\n"
"   gl_Position = u_MVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSrc =
"#version 330 core\n"
"in vec2 vUV;\n"
"out vec4 FragColor;\n"
"uniform vec3 u_Color;\n"
"uniform sampler2D u_Tex;\n"
"uniform bool u_UseTex;\n"
"void main() {\n"
"   vec3 c = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;\n"
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
    std::uniform_real_distribution<float> pickPos(-8.f, 8.f);
    float scale = 1.0f + 0.15f * (floorNum - 1);

    for (auto& e : enemies) {
        const EnemyType& t = enemyTypes[pickType(rng)];
        int hp = (int)(t.hp * scale);
        e.stats = Fighter{ t.name, hp, hp, (int)(t.atk * scale), t.def, 0, 0 };
        e.body->color = t.color;
        Vector3 p;
        do { p = Vector3(pickPos(rng), 0, pickPos(rng)); } while (std::sqrt(p.X * p.X + p.Z * p.Z) < 4.0f);
        e.body->setPosition(p);
    }
}

void resetRun() {
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

void initScene() {
    rootScene = std::make_shared<Node>();

    floorNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    floorNode->setScale(Vector3(20, 0.2f, 20));
    floorNode->setPosition(Vector3(0, -0.6f, 0));
    floorNode->color = Vector3(0.3f, 0.7f, 0.3f);
    rootScene->addChild(floorNode);

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
    }
    rootScene->addChild(player);

    playerHpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    playerHpBar->color = Vector3(0.1f, 0.9f, 0.1f);
    rootScene->addChild(playerHpBar);

    for (int i = 0; i < 3; i++) {
        Enemy e;
        e.body = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
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
        if (k == '1') battle.playerAct(PlayerAction::Attack);
        if (k == '2') battle.playerAct(PlayerAction::Skill);
        if (k == '3') battle.playerAct(PlayerAction::Defend);
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
        hud.text(18, 88, battle.playerTurn ? "YOUR TURN" : "ENEMY TURN", glm::vec3(1.f, 0.8f, 0.3f));
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
    bar->setPosition(Vector3(base.X, base.Y + 1.0f, base.Z));
}

void updateGame(float dt) {
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
                battle.start(&playerStats, &enemies[i].stats);
                state = GameState::Battle;
                break;
            }
        }
    }
    else if (state == GameState::Battle) {
        battle.update(dt);
        if (battle.finished) {
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
        if (e.stats.alive()) {
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
