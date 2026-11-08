#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <string>

// Depth-only render target for the sun. Render the world into it with `program`,
// then sample `texture` from the main shader to find out what is in shadow.
class ShadowMap {
public:
    static constexpr int Size = 2048;
    GLuint program = 0; // depth-only shader; uses u_MVP like the main shader
    GLuint texture = 0;

    // Unit vector pointing from the scene towards the sun.
    static glm::vec3 sunDirection() { return glm::normalize(glm::vec3(-0.4f, 1.f, 0.6f)); }

    // Orthographic view-projection of the sun, wide enough for the whole arena.
    static glm::mat4 sunMatrix() {
        const glm::vec3 eye = sunDirection() * 25.f;
        const glm::mat4 view = glm::lookAt(eye, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        const glm::mat4 projection = glm::ortho(-16.f, 16.f, -16.f, 16.f, 1.f, 50.f);
        return projection * view;
    }

    void init() {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Size, Size, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Outside the map counts as fully lit (depth 1.0).
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        const float border[4] = { 1.f, 1.f, 1.f, 1.f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
        glDrawBuffer(GL_NONE); // depth only
        glReadBuffer(GL_NONE);
        const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (!complete) throw std::runtime_error("Shadow map framebuffer is incomplete");

        const GLuint vs = compile(GL_VERTEX_SHADER,
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "uniform mat4 u_MVP;\n"
            "void main() { gl_Position = u_MVP * vec4(aPos, 1.0); }\n");
        const GLuint fs = compile(GL_FRAGMENT_SHADER,
            "#version 330 core\n"
            "void main() {}\n");
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) throw std::runtime_error("Shadow shader linking failed");
    }

    // Start the depth pass: render the world with `program` and sunMatrix() as the view-projection.
    void begin() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, Size, Size);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL); // reduces self-shadowing stripes
        glPolygonOffset(2.f, 4.f);
        glUseProgram(program);
    }

    // Back to the window; pass its current size.
    void end(int windowWidth, int windowHeight) {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);
    }

private:
    GLuint fbo = 0;

    static GLuint compile(GLenum type, const char* source) {
        const GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            char log[1024] = {};
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            glDeleteShader(shader);
            throw std::runtime_error(std::string("Shadow shader compilation failed: ") + log);
        }
        return shader;
    }
};
