#pragma once
#include <string>

// returns the OpenGL texture ID of the loaded texture, or 0 if failed
unsigned int loadTexture(const std::string& path);