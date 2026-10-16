
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




GLuint shaderProgram = 0;
GLuint cubeVAO = 0;
int cubeIndexCount = 36;

std::shared_ptr<Node> rootScene;
std::shared_ptr<MeshNode> sunNode;
std::shared_ptr<MeshNode> earthNode;

const char* vertexShaderSrc = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 u_MVP;\n"
"void main()\n"
"{\n"
"   gl_Position = u_MVP * vec4(aPos, 1.0);\n"
"}\n";

const char* fragmentShaderSrc = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0, 0.6, 0.1, 1.0);\n" 
"}\n";

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

    sunNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    sunNode->setScale(Vector3(1.5f, 1.5f, 1.5f));
    rootScene->addChild(sunNode);

    earthNode = std::make_shared<MeshNode>(cubeVAO, cubeIndexCount);
    earthNode->setPosition(Vector3(4.0f, 0.0f, 0.0f)); 
    earthNode->setScale(Vector3(0.5f, 0.5f, 0.5f));
    sunNode->addChild(earthNode);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 viewProj = proj * view;

    static float angle = 0.0f;
    angle += 0.8f;

    
    sunNode->setRotation(Vector3(0.0f, angle, 0.0f));
    earthNode->setRotation(Vector3(0.0f, angle * 2.0f, 0.0f));

    rootScene->update(0.016f);

    rootScene->render(shaderProgram, viewProj);

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
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}