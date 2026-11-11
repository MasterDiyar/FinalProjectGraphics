#include <glad/glad.h>
#include <GL/freeglut.h>
#include "../Game/DissolveShader.h"
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

// Real-driver check: a hidden GL window renders into an offscreen framebuffer.
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitWindowSize(64, 64);
    int window = glutCreateWindow("Dissolve shader validation");
    glutHideWindow();
    if (!gladLoadGL()) return 1;
    // Some GL loaders probe legacy extension strings in a core context.
    while (glGetError() != GL_NO_ERROR) {}
    GLuint program = 0;
    try { program = loadDissolveProgram(); }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
    constexpr int size = 128;
    GLuint framebuffer, color, depth, sun, vao, vbo;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return 2;
    glGenTextures(1, &sun);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sun);
    const float lit = 1.f;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &lit);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    const float vertices[] = {-0.8f,-0.8f,0, 0.8f,-0.8f,0, 0.8f,0.8f,0,
                              -0.8f,-0.8f,0, 0.8f,0.8f,0, -0.8f,0.8f,0};
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glUseProgram(program);
    const float identity[] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    for (const char* name : {"u_MVP", "u_Model", "u_LightVP"})
        glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, identity);
    glUniform3f(glGetUniformLocation(program, "u_Color"), 0.25f, 0.5f, 0.8f);
    glUniform3f(glGetUniformLocation(program, "u_SunDir"), 0, 0, 1);
    glUniform3f(glGetUniformLocation(program, "u_DissolveOrigin"), 0, 0, 0);
    glUniform1i(glGetUniformLocation(program, "u_ShadowMap"), 1);
    glUniform1i(glGetUniformLocation(program, "u_Tex"), 0);
    glViewport(0, 0, size, size);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    std::vector<unsigned char> pixels(size * size * 4);
    std::vector<float> depths(size * size), shadowDepths(size * size);
    int previous = size * size;
    bool sawEmber = false;
    for (float progress : {0.f, 0.35f, 0.65f, 1.f}) {
        glUniform1f(glGetUniformLocation(program, "u_DissolveProgress"), progress);
        glUniform1i(glGetUniformLocation(program, "u_DepthOnly"), GL_FALSE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glReadPixels(0, 0, size, size, GL_DEPTH_COMPONENT, GL_FLOAT, depths.data());
        int visible = 0;
        for (int i = 0; i < size * size; ++i) {
            if (pixels[i * 4 + 3]) ++visible;
            if (progress > 0.f && pixels[i * 4] > 220 && pixels[i * 4 + 2] < 50) sawEmber = true;
        }
        std::cout << "Progress " << progress << ": " << visible << " visible pixels\n";
        if ((progress < 1.f && visible == 0) || visible >= previous || (progress == 1.f && visible != 0)) return 3;
        previous = visible;
        glUniform1i(glGetUniformLocation(program, "u_DepthOnly"), GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glReadPixels(0, 0, size, size, GL_DEPTH_COMPONENT, GL_FLOAT, shadowDepths.data());
        for (int i = 0; i < size * size; ++i)
            if (std::abs(depths[i] - shadowDepths[i]) > 0.00001f) return 4;
    }
    GLenum error = glGetError();
    if (!sawEmber || error != GL_NO_ERROR) {
        std::cerr << "Ember edge: " << sawEmber << ", GL error: " << error << '\n';
        return 5;
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &depth);
    glDeleteTextures(1, &color);
    glDeleteTextures(1, &sun);
    glDeleteProgram(program);
    glutDestroyWindow(window);
    std::cout << "Dissolve GLSL, shrinking coverage, ember edge and shadow mask checks passed\n";
}
