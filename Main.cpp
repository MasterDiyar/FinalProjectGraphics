
#include "Modules/Vector3.h" 
#include "Modules/Node.h"
#include "Modules/MeshNode.h"

#include <glad/glad.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include "Game/Battle.h"
#include <cmath>



GLuint shaderProgram = 0;
GLuint cubeVAO = 0;
int cubeIndexCount = 36;

std::shared_ptr<Node> rootScene;

GameState state = GameState::Explore;
Battle battle;
Fighter playerStats{ "Hero", 40, 40, 9, 2, 10, 10 };
Fighter enemyStats{ "Slime", 25, 25, 7, 1, 0, 0 };

std::shared_ptr<MeshNode> player, enemy, floorNode, enemyHpBar, playerHpBar;
bool keys[256] = {};
int lastTime = 0;

const char* vertexShaderSrc = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 u_MVP;\n"
"void main()\n"
"{\n"
"   gl_Position = u_MVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSrc = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 u_Color;\n"
"void main() { FragColor = vec4(u_Color, 1.0); }\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
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

void initScene() {
    rootScene = std::make_shared<Node>();

    floorNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    floorNode->setScale(Vector3(20, 0.2f, 20));
    floorNode->setPosition(Vector3(0, -0.6f, 0));
    floorNode->color = Vector3(0.3f, 0.7f, 0.3f);
    rootScene->addChild(floorNode);

    player = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    player->color = Vector3(0.2f, 0.5f, 1.0f);
    rootScene->addChild(player);

    enemy = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    enemy->setPosition(Vector3(5, 0, -4));
    enemy->color = Vector3(0.9f, 0.2f, 0.2f);
    rootScene->addChild(enemy);

    playerHpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    playerHpBar->color = Vector3(0.1f, 0.9f, 0.1f);
    rootScene->addChild(playerHpBar);

    enemyHpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    enemyHpBar->color = Vector3(0.9f, 0.1f, 0.1f);
    rootScene->addChild(enemyHpBar);
}

void keyDown(unsigned char k, int, int) {
    keys[k] = true;
    if (state == GameState::Battle) {
        if (k == '1') battle.playerAct(PlayerAction::Attack);
        if (k == '2') battle.playerAct(PlayerAction::Skill);
        if (k == '3') battle.playerAct(PlayerAction::Defend);
    }
    if ((state == GameState::Victory || state == GameState::Defeat) && k == ' ') {
        // roguelike restart, reset player and enemy stats, and return to exploration
        if (state == GameState::Defeat) { playerStats.hp = playerStats.maxHp; playerStats.mp = playerStats.maxMp; }
        enemyStats.hp = enemyStats.maxHp;
        enemy->setPosition(Vector3(5, 0, -4));
        player->setPosition(Vector3(0, 0, 0));
        state = GameState::Explore;
    }
}
void keyUp(unsigned char k, int, int) { keys[k] = false; }

void setBar(std::shared_ptr<MeshNode>& bar, Vector3 base, int hp, int maxHp) {
    float r = (float)hp / maxHp;
    bar->setScale(Vector3(std::max(r, 0.001f) * 1.5f, 0.15f, 0.15f));
    bar->setPosition(Vector3(base.X, base.Y + 1.0f, base.Z));
}

void updateGame(float dt) {
    if (state == GameState::Explore) {
        Vector3 p = player->getPosition();
        float speed = 4.0f * dt;
        if (keys['w']) p.Z -= speed;
        if (keys['s']) p.Z += speed;
        if (keys['a']) p.X -= speed;
        if (keys['d']) p.X += speed;
        player->setPosition(p);

        Vector3 e = enemy->getPosition();
        float dx = p.X - e.X, dz = p.Z - e.Z;
        if (std::sqrt(dx * dx + dz * dz) < 1.3f) {
            battle.start(&playerStats, &enemyStats);
            state = GameState::Battle;
        }
    }
    else if (state == GameState::Battle) {
        battle.update(dt);
        if (battle.finished)
            state = battle.playerWon ? GameState::Victory : GameState::Defeat;
    }

    setBar(playerHpBar, player->getPosition(), playerStats.hp, playerStats.maxHp);
    setBar(enemyHpBar, enemy->getPosition(), enemyStats.hp, enemyStats.maxHp);
    enemy->setScale(enemyStats.alive() ? Vector3(1, 1, 1) : Vector3(0, 0, 0));
    enemyHpBar->setScale(enemyStats.alive() ? enemyHpBar->getScale() : Vector3(0, 0, 0));

    // Update window title with game state and player stats
    std::string t = "HP " + std::to_string(playerStats.hp) + "/" + std::to_string(playerStats.maxHp)
        + " MP " + std::to_string(playerStats.mp);
    if (state == GameState::Battle) t += " | [1]Attack [2]Skill [3]Defend | " + battle.log;
    else if (state == GameState::Victory) t += " | VICTORY - SPACE to continue";
    else if (state == GameState::Defeat) t += " | DEFEAT - SPACE to restart";
    else t += " | WASD to move, touch the red cube";
    glutSetWindowTitle(t.c_str());
}

void display() {
    glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    updateGame(dt);

    Vector3 p = player->getPosition();
    glm::mat4 view = glm::lookAt(glm::vec3(p.X, p.Y + 7.0f, p.Z + 9.0f), glm::vec3(p.X, p.Y, p.Z), glm::vec3(0, 1, 0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    rootScene->update(dt);
    rootScene->render(shaderProgram, proj * view);
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
    glutCreateWindow("OpenGL Scene Graph - Test Sun");

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    initShaders();
    initCube();
    initScene();

    glutDisplayFunc(display);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}