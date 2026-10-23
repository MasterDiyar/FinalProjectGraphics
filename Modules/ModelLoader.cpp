#include "ModelLoader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <array>
#include <iostream>

// OBJ indexes 1-based, negative indexes are relative to the end of the list, 0 is invalid
static int fixIndex(int i, int size) {
    if (i > 0) return i - 1;
    if (i < 0) return size + i;
    return -1;
}

// v//vn or v/vt/vn or v/vt or v
static void parseVertex(const std::string& tok, int& v, int& t, int& n) {
    v = t = n = 0;
    std::vector<std::string> parts;
    std::stringstream ss(tok);
    std::string p;
    while (std::getline(ss, p, '/')) parts.push_back(p);
	// "v//vn" check for empty middle part
    if (parts.size() > 0 && !parts[0].empty()) v = std::stoi(parts[0]);
    if (parts.size() > 1 && !parts[1].empty()) t = std::stoi(parts[1]);
    if (parts.size() > 2 && !parts[2].empty()) n = std::stoi(parts[2]);
}

Model loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "OBJ: cannot open " << path << "\n";
        return {};
    }

    std::vector<glm::vec3> positions, normals;
    std::vector<glm::vec2> uvs;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::map<std::array<int, 3>, unsigned int> cache;

    auto getIndex = [&](int vi, int ti, int ni) -> unsigned int {
        std::array<int, 3> key{ vi, ti, ni };
        auto it = cache.find(key);
        if (it != cache.end()) return it->second;

        glm::vec3 p = positions[vi];
        glm::vec3 n = ni >= 0 ? normals[ni] : glm::vec3(0, 1, 0);
        glm::vec2 t = ti >= 0 ? uvs[ti] : glm::vec2(0);
        float data[8] = { p.x, p.y, p.z, n.x, n.y, n.z, t.x, t.y };
        vertices.insert(vertices.end(), data, data + 8);

        unsigned int idx = (unsigned int)(vertices.size() / 8 - 1);
        cache[key] = idx;
        return idx;
        };

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") { glm::vec3 p; ss >> p.x >> p.y >> p.z; positions.push_back(p); }
        else if (type == "vn") { glm::vec3 n; ss >> n.x >> n.y >> n.z; normals.push_back(n); }
        else if (type == "vt") { glm::vec2 t; ss >> t.x >> t.y; uvs.push_back(t); }
        else if (type == "f") {
            std::vector<unsigned int> face;
            std::string tok;
            while (ss >> tok) {
                int v, t, n;
                parseVertex(tok, v, t, n);
                face.push_back(getIndex(fixIndex(v, (int)positions.size()),
                    fixIndex(t, (int)uvs.size()),
                    fixIndex(n, (int)normals.size())));
            }
			// triangulate polygonal face (fan triangulation)
            for (size_t i = 1; i + 1 < face.size(); i++) {
                indices.push_back(face[0]);
                indices.push_back(face[i]);
                indices.push_back(face[i + 1]);
            }
        }
    }

    if (indices.empty()) {
        std::cerr << "OBJ: no faces in " << path << "\n";
        return {};
    }

    Model m;
    m.indexCount = (int)indices.size();
    unsigned int vbo, ebo;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return m;
}