#include "../Modules/Vector3.h"
#include "../Modules/Node.h"
#include "../Modules/MeshNode.h"
#include "../Modules/ModelLoader.h"
#include "../Modules/Texture.h"
#include "../Modules/ArenaNode.h"
#include "../Modules/PlayerNode.h"
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include "Battle.h"
#include "HudNode.h"
#include "EnemyNode.h"
#include "ParticleNode.h"
#include "ShadowMap.h"
#include "GameApp.h"
#include "SpawnLayout.h"
#include <cmath>
#include <random>
#include <string>
#include <cctype>
#include <stdexcept>

namespace {

    GLuint shaderProgram = 0;
    GLuint cubeVAO = 0;
    int cubeIndexCount = 36;

    std::shared_ptr<Node> rootScene;
    std::shared_ptr<Node> worldScene; // everything that casts shadows; the HUD stays outside
    ShadowMap shadowMap;
    GameState state = GameState::Explore;
    Battle battle;
    std::shared_ptr<HudNode> hudNode;
    std::shared_ptr<ParticleNode> particles;

    std::vector<std::shared_ptr<EnemyNode>> enemies;
    int currentEnemy = -1;
    int floorNum = 1;
    int kills = 0;
    int metaAttackBonus = 0;

    std::shared_ptr<MeshNode> floorNode, playerHpBar;
    std::shared_ptr<PlayerNode> player;
    bool keys[256] = {};
    bool paused = false;
    int lastTime = 0;
    int windowWidth = 800, windowHeight = 600;
    int mouseX = -1, mouseY = -1; // last known mouse position, -1 when outside the window
    constexpr float arenaLimit = 9.5f;
    glm::vec3 cameraEye(0.f, 7.f, 9.f);
    glm::vec3 cameraTarget(0.f);
    float cameraAngle = 0.0f; // Угол поворота камеры на Q и E
    float cameraShake = 0.f, shakeTime = 0.f; // hit shake strength (0-1) and its phase
    glm::vec3 shakeOffset(0.f);

    // World-space position of a node. The hero's body is a child of PlayerNode while enemy
    // bodies sit directly in the scene, so their local getPosition() values are not comparable.
    Vector3 worldPosition(const std::shared_ptr<MeshNode>& node) {
        const glm::mat4 m = node->getWorldTransform();
        return Vector3(m[3].x, m[3].y, m[3].z);
    }

    struct CombatMotion {
        std::shared_ptr<MeshNode> actor;
        Vector3 direction;
        float attackTime = 0.f, hitTime = 0.f, guardTime = 0.f;
        float reach = 0.4f;

        bool active() const { return attackTime > 0.f || hitTime > 0.f || guardTime > 0.f; }
        void strike(const std::shared_ptr<MeshNode>& source, const std::shared_ptr<MeshNode>& target, bool skill = false) {
            actor = source;
            Vector3 a = worldPosition(source), b = worldPosition(target);
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

    void hitBurst(Vector3 where, Vector3 color, int count, float shake) {
        if (particles) particles->burst(Vector3(where.X, where.Y + 0.6f, where.Z), color, count);
        cameraShake = std::max(cameraShake, shake);
    }

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
        "uniform float u_Alpha;\n"
        "uniform vec3 u_SunDir;\n"
        "uniform mat4 u_LightVP;\n"
        "uniform sampler2D u_ShadowMap;\n"
        "float shadowAt(vec3 world, vec3 n) {\n"
        "   vec4 p = u_LightVP * vec4(world + n * 0.05, 1.0);\n"
        "   vec3 q = p.xyz / p.w * 0.5 + 0.5;\n"
        "   if (q.z > 1.0) return 0.0;\n"
        "   vec2 texel = 1.0 / vec2(textureSize(u_ShadowMap, 0));\n"
        "   float shadow = 0.0;\n"
        "   for (int x = -1; x <= 1; ++x)\n"
        "       for (int y = -1; y <= 1; ++y)\n"
        "           shadow += (q.z - 0.001 > textureLod(u_ShadowMap, q.xy + vec2(x, y) * texel, 0.0).r) ? 1.0 : 0.0;\n"
        "   return shadow / 9.0;\n"
        "}\n"
        "void main() {\n"
        "   vec3 c = u_UseTex ? texture(u_Tex, vUV).rgb * u_Color : u_Color;\n"
        "   c = mix(c, vec3(1.0, 0.25, 0.2), clamp(u_HitFlash, 0.0, 1.0));\n"
        "   if (u_Lighting) {\n"
        "       vec3 n = normalize(cross(dFdx(vWorld), dFdy(vWorld)));\n"
        "       float diffuse = max(dot(n, normalize(u_SunDir)), 0.0);\n"
        "       c *= 0.45 + 0.55 * diffuse * (1.0 - 0.8 * shadowAt(vWorld, n));\n"
        "   }\n"
        "   FragColor = vec4(c, u_Alpha);\n"
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
            const auto& p = spawnPoints.at(spawnIndex++);
            e->spawn(pickType(rng), scale, Vector3(p.x, 0, p.z));
        }
    }

    void resetRun() {
        playerMotion.clear();
        enemyMotion.clear();
        floorNum = 1;
        kills = 0;
        player->stats = Fighter{ "Hero", 40, 40, 9 + metaAttackBonus, 2, 10, 10 };
        spawnEnemies();
        player->setPosition(Vector3(0, 0, 0));
        state = GameState::Explore;
    }

    void finishUpgrade() {
        bool allDead = true;
        for (auto& e : enemies) if (e->stats.alive()) allDead = false;

        if (allDead) {
            floorNum++;
            spawnEnemies();
            player->setPosition(Vector3(0, 0, 0));
        }
        state = GameState::Explore;
    }

    void initScene() {
        rootScene = std::make_shared<Node>();
        worldScene = std::make_shared<Node>();
        rootScene->addChild(worldScene);

        auto arena = std::make_shared<ArenaNode>(cubeVAO, cubeIndexCount);
        worldScene->addChild(arena);

        player = std::make_shared<PlayerNode>(cubeVAO, cubeIndexCount);
        worldScene->addChild(player);

        playerHpBar = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
        playerHpBar->color = Vector3(0.1f, 0.9f, 0.1f);
        worldScene->addChild(playerHpBar);

        for (int i = 0; i < 3; ++i) {
            auto enemy = std::make_shared<EnemyNode>(cubeVAO, cubeIndexCount);
            worldScene->addChild(enemy);
            enemies.push_back(enemy);
        }
        spawnEnemies();
        particles = std::make_shared<ParticleNode>(cubeVAO, cubeIndexCount);
        worldScene->addChild(particles);
        hudNode = std::make_shared<HudNode>();
        hudNode->init();
        rootScene->addChild(hudNode);
    }

    void chooseAction(unsigned char k) {
        if (paused) return;
        if (state == GameState::Battle) {
            if (!battle.playerTurn || battle.finished || playerMotion.active() || enemyMotion.active()) return;
            if (k < '1' || k > '3') return;
            auto target = enemies[currentEnemy]->body;
            int hpBefore = enemies[currentEnemy]->stats.hp;
            if (k == '1') battle.playerAct(PlayerAction::Attack);
            if (k == '2') battle.playerAct(PlayerAction::Skill);
            if (k == '3') {
                battle.playerAct(PlayerAction::Defend);
                playerMotion.actor = player->getBody();
                playerMotion.guardTime = 0.36f;
            }
            if (enemies[currentEnemy]->stats.hp < hpBefore) {
                playerMotion.strike(player->getBody(), target, k == '2');
                enemyMotion.actor = target;
                enemyMotion.hitTime = 0.3f;
                hitBurst(target->getPosition(), k == '2' ? Vector3(0.5f, 0.8f, 1.f) : Vector3(1.f, 0.85f, 0.35f),
                    k == '2' ? 26 : 14, k == '2' ? 0.8f : 0.5f);
            }
        }
        else if (state == GameState::Upgrade) {
            if (k == '1')      player->stats.attack += 2;
            else if (k == '2') { player->stats.maxHp += 8; player->stats.hp = player->stats.maxHp; }
            else if (k == '3') { player->stats.maxMp += 4; player->stats.mp = player->stats.maxMp; }
            else return;
            finishUpgrade();
        }
        else if (state == GameState::Defeat && k == ' ') {
            resetRun();
        }
    }

    void keyDown(unsigned char k, int, int) {
        k = static_cast<unsigned char>(std::tolower(k));
        if (k == 'p') {
            paused = !paused;
            std::fill(std::begin(keys), std::end(keys), false);
            return;
        }
        if (paused) return;
        keys[k] = true;
        chooseAction(k);
    }

    void refreshHud();

    void mouseDown(int button, int pressed, int x, int y) {
        if (button != GLUT_LEFT_BUTTON || pressed != GLUT_DOWN) return;
        refreshHud();
        if (auto action = hudNode->actionAt(x, y, windowWidth, windowHeight)) chooseAction(action);
    }

    size_t enemiesRemaining() {
        return static_cast<size_t>(std::count_if(enemies.begin(), enemies.end(),
            [](const std::shared_ptr<EnemyNode>& enemy) { return enemy->stats.alive(); }));
    }

    void refreshHud() {
        HudState snapshot;
        snapshot.state = state;
        snapshot.player = player->stats;
        if (currentEnemy >= 0) snapshot.enemy = enemies[currentEnemy]->stats;
        snapshot.floor = floorNum;
        snapshot.kills = kills;
        snapshot.metaAttackBonus = metaAttackBonus;
        snapshot.remaining = enemiesRemaining();
        snapshot.total = enemies.size();
        snapshot.paused = paused;
        snapshot.resolving = playerMotion.active() || enemyMotion.active();
        snapshot.playerTurn = battle.playerTurn;
        snapshot.finished = battle.finished;
        snapshot.log = battle.log;
        snapshot.hover = snapshot.buttonAt(mouseX, mouseY, windowWidth, windowHeight);
        static int lastHover = -1;
        if (snapshot.hover != lastHover) {
            lastHover = snapshot.hover;
            glutSetCursor(lastHover >= 0 ? GLUT_CURSOR_INFO : GLUT_CURSOR_LEFT_ARROW);
        }
        hudNode->setState(snapshot);
    }

    void keyUp(unsigned char k, int, int) {
        keys[static_cast<unsigned char>(std::tolower(k))] = false;
    }

    void mouseMove(int x, int y) {
        mouseX = x;
        mouseY = y;
    }

    void mouseEntry(int entryState) {
        if (entryState == GLUT_LEFT) mouseX = mouseY = -1;
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
        if (paused) return;
        shakeTime += dt;
        cameraShake = std::max(0.f, cameraShake - 3.f * dt);
        shakeOffset = glm::vec3(std::sin(shakeTime * 90.f), std::sin(shakeTime * 71.f + 1.3f), 0.f)
            * (cameraShake * 0.15f);
        playerMotion.update(dt);
        enemyMotion.update(dt);
        if (state == GameState::Explore) {
            player->handleMovement(keys, dt);
            Vector3 p = player->getPosition();

            for (size_t i = 0; i < enemies.size(); i++) {
                if (!enemies[i]->stats.alive()) continue;
                Vector3 e = enemies[i]->body->getPosition();
                float dx = p.X - e.X, dz = p.Z - e.Z;
                if (std::sqrt(dx * dx + dz * dz) < 1.3f) {
                    currentEnemy = (int)i;
                    playerMotion.clear();
                    enemyMotion.clear();
                    battle.start(&player->stats, &enemies[i]->stats);
                    state = GameState::Battle;
                    break;
                }
            }
        }
        else if (state == GameState::Battle) {
            int hpBefore = player->stats.hp;
            battle.update(dt);
            if (player->stats.hp < hpBefore) {
                enemyMotion.strike(enemies[currentEnemy]->body, player->getBody());
                playerMotion.actor = player->getBody();
                playerMotion.hitTime = 0.3f;
                hitBurst(player->getPosition(), Vector3(0.95f, 0.25f, 0.2f), 14, 1.f);
            }
            if (battle.finished && !playerMotion.active() && !enemyMotion.active()) {
                if (battle.playerWon) {
                    kills++;
                    player->stats.mp = std::min(player->stats.maxMp, player->stats.mp + 3);
                    state = GameState::Upgrade;
                }
                else {
                    metaAttackBonus++;
                    state = GameState::Defeat;
                }
            }
        }

        setBar(playerHpBar, player->getPosition(), player->stats.hp, player->stats.maxHp);

        for (auto& e : enemies)
            e->keepVisibleForFeedback(e->body == enemyMotion.actor && (enemyMotion.active() || playerMotion.active()));
    }

    glm::mat4 updateCamera(float dt) {
        // Управление вращением камеры через клавиши Q и E
        if (keys['q']) cameraAngle += 2.0f * dt;
        if (keys['e']) cameraAngle -= 2.0f * dt;

        Vector3 position = player->getPosition();
        glm::vec3 hero(position.X, position.Y, position.Z);
        glm::vec3 target = hero;

        glm::vec3 baseOffset(0.f, 7.f, 9.f);
        float cosA = std::cos(cameraAngle);
        float sinA = std::sin(cameraAngle);
        glm::vec3 rotatedOffset(
            baseOffset.x * cosA - baseOffset.z * sinA,
            baseOffset.y,
            baseOffset.x * sinA + baseOffset.z * cosA
        );

        glm::vec3 eye = hero + rotatedOffset;

        if (state == GameState::Battle && currentEnemy >= 0) {
            Vector3 enemyPosition = enemies[currentEnemy]->body->getPosition();
            glm::vec3 enemy(enemyPosition.X, enemyPosition.Y, enemyPosition.Z);
            target = (hero + enemy) * 0.5f + glm::vec3(0, 0.4f, 0);
        }

        float blend = paused ? 0.f : 1.f - std::exp(-6.f * dt);
        cameraEye = glm::mix(cameraEye, eye, blend);
        cameraTarget = glm::mix(cameraTarget, target, blend);
        return glm::lookAt(cameraEye + shakeOffset, cameraTarget, glm::vec3(0, 1, 0));
    }

    void display() {
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        int now = glutGet(GLUT_ELAPSED_TIME);
        float dt = std::clamp((now - lastTime) / 1000.0f, 0.0f, 0.1f);
        lastTime = now;

        updateGame(dt);

        glm::mat4 view = updateCamera(dt);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), float(windowWidth) / windowHeight, 0.1f, 100.0f);

        refreshHud();
        rootScene->update(paused ? 0.f : dt);
        shadowMap.begin();
        worldScene->render(shadowMap.program, ShadowMap::sunMatrix());
        shadowMap.end(windowWidth, windowHeight);
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_Lighting"), GL_TRUE);
        glUniform1f(glGetUniformLocation(shaderProgram, "u_Alpha"), 1.f);
        const glm::vec3 sunDirection = ShadowMap::sunDirection();
        const glm::mat4 sunMatrix = ShadowMap::sunMatrix();
        glUniform3fv(glGetUniformLocation(shaderProgram, "u_SunDir"), 1, &sunDirection[0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_LightVP"), 1, GL_FALSE, &sunMatrix[0][0]);
        glUniform1i(glGetUniformLocation(shaderProgram, "u_ShadowMap"), 1);
        glActiveTexture(GL_TEXTURE1); // unit 0 stays free for material textures
        glBindTexture(GL_TEXTURE_2D, shadowMap.texture);
        glActiveTexture(GL_TEXTURE0);
        rootScene->render(shaderProgram, proj * view);
        glActiveTexture(GL_TEXTURE1); // release the shadow map before the next depth pass
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glutSwapBuffers();
    }

    void timer(int) {
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
    }

} // namespace

int runGame(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Monster Clash");

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    try {
        initShaders();
        shadowMap.init();
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
    initCube();
    initScene();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc(mouseDown);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);
    glutEntryFunc(mouseEntry);
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
