#pragma once
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

inline GLuint loadDissolveProgram() {
    auto read = [](const char* path) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error(std::string("Cannot open dissolve shader: ") + path);
        std::ostringstream text;
        text << file.rdbuf();
        return text.str();
    };
    auto compile = [](GLenum type, const std::string& source) {
        GLuint stage = glCreateShader(type);
        const char* data = source.c_str();
        glShaderSource(stage, 1, &data, nullptr);
        glCompileShader(stage);
        GLint ok = GL_FALSE;
        glGetShaderiv(stage, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[4096] = {};
            glGetShaderInfoLog(stage, sizeof(log), nullptr, log);
            glDeleteShader(stage);
            throw std::runtime_error(std::string("Dissolve shader compile failed: ") + log);
        }
        return stage;
    };
    const auto vertex = read("shaders/enemy_dissolve.vert");
    const auto fragment = read("shaders/enemy_dissolve.frag");
    GLuint vs = 0, fs = 0, program = 0;
    try {
        vs = compile(GL_VERTEX_SHADER, vertex);
        fs = compile(GL_FRAGMENT_SHADER, fragment);
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        GLint ok = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[4096] = {};
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            throw std::runtime_error(std::string("Dissolve shader link failed: ") + log);
        }
    } catch (...) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        if (program) glDeleteProgram(program);
        throw;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
