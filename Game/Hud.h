#pragma once

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cctype>
#include <string>
#include <vector>

// A tiny bitmap font drawn as triangles: no extra UI or font dependency.
class Hud {
    GLuint vao = 0, vbo = 0, program = 0;
    std::vector<float> vertices;

    static std::array<unsigned char, 7> glyph(char c) {
        switch (std::toupper(static_cast<unsigned char>(c))) {
        case 'A': return {14,17,17,31,17,17,17};
        case 'B': return {30,17,17,30,17,17,30};
        case 'C': return {14,17,16,16,16,17,14};
        case 'D': return {30,17,17,17,17,17,30};
        case 'E': return {31,16,16,30,16,16,31};
        case 'F': return {31,16,16,30,16,16,16};
        case 'G': return {14,17,16,23,17,17,15};
        case 'H': return {17,17,17,31,17,17,17};
        case 'I': return {14,4,4,4,4,4,14};
        case 'J': return {7,2,2,2,18,18,12};
        case 'K': return {17,18,20,24,20,18,17};
        case 'L': return {16,16,16,16,16,16,31};
        case 'M': return {17,27,21,21,17,17,17};
        case 'N': return {17,25,21,19,17,17,17};
        case 'O': return {14,17,17,17,17,17,14};
        case 'P': return {30,17,17,30,16,16,16};
        case 'Q': return {14,17,17,17,21,18,13};
        case 'R': return {30,17,17,30,20,18,17};
        case 'S': return {15,16,16,14,1,1,30};
        case 'T': return {31,4,4,4,4,4,4};
        case 'U': return {17,17,17,17,17,17,14};
        case 'V': return {17,17,17,17,17,10,4};
        case 'W': return {17,17,17,21,21,21,10};
        case 'X': return {17,17,10,4,10,17,17};
        case 'Y': return {17,17,10,4,4,4,4};
        case 'Z': return {31,1,2,4,8,16,31};
        case '0': return {14,17,19,21,25,17,14};
        case '1': return {4,12,4,4,4,4,14};
        case '2': return {14,17,1,2,4,8,31};
        case '3': return {30,1,1,14,1,1,30};
        case '4': return {2,6,10,18,31,2,2};
        case '5': return {31,16,16,30,1,1,30};
        case '6': return {14,16,16,30,17,17,14};
        case '7': return {31,1,2,4,8,8,8};
        case '8': return {14,17,17,14,17,17,14};
        case '9': return {14,17,17,15,1,1,14};
        case '/': return {1,2,2,4,8,8,16};
        case '+': return {0,4,4,31,4,4,0};
        case '-': return {0,0,0,31,0,0,0};
        case ':': return {0,4,4,0,4,4,0};
        case '!': return {4,4,4,4,4,0,4};
        case '[': return {14,8,8,8,8,8,14};
        case ']': return {14,2,2,2,2,2,14};
        default: return {};
        }
    }

    void quad(float x, float y, float w, float h) {
        const float data[] = {x,y,0, x+w,y,0, x+w,y+h,0,
                              x,y,0, x+w,y+h,0, x,y+h,0};
        vertices.insert(vertices.end(), std::begin(data), std::end(data));
    }

    void flush(glm::vec3 color) {
        glUniform3fv(glGetUniformLocation(program, "u_Color"), 1, &color[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 3));
        vertices.clear();
    }

public:
    // Coordinates use an 800 x 600 canvas, shared with mouse hit testing.
    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void begin(GLuint shader) {
        program = shader;
        glDisable(GL_DEPTH_TEST);
        glUseProgram(program);
        glBindVertexArray(vao);
        const glm::mat4 projection = glm::ortho(0.f, 800.f, 600.f, 0.f, -1.f, 1.f);
        glUniformMatrix4fv(glGetUniformLocation(program, "u_MVP"), 1, GL_FALSE, &projection[0][0]);
        glUniform1i(glGetUniformLocation(program, "u_UseTex"), GL_FALSE);
        glUniform1i(glGetUniformLocation(program, "u_Lighting"), GL_FALSE);
        glUniform1f(glGetUniformLocation(program, "u_HitFlash"), 0.f);
    }

    void rect(float x, float y, float w, float h, glm::vec3 color) {
        quad(x, y, w, h);
        flush(color);
    }

    void text(float x, float y, const std::string& value, glm::vec3 color = glm::vec3(0.94f)) {
        for (char c : value) {
            const auto rows = glyph(c);
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (rows[row] & (1 << (4 - col))) quad(x + col * 2.f, y + row * 2.f, 2, 2);
            x += 12.f;
        }
        flush(color);
    }

    void end() {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnable(GL_DEPTH_TEST);
    }
};
