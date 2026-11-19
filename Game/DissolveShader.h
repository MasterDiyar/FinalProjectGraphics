#pragma once
#include "ShaderProgram.h"
inline GLuint loadDissolveProgram() { return loadShaderProgram("shaders/enemy_dissolve.vert", "shaders/enemy_dissolve.frag"); }
