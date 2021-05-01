#pragma once

#include <filesystem>

const std::filesystem::path _getShaderDir();
std::filesystem::path GetShaderPath(std::string name);

bool GetShader(std::string name, D3D12_SHADER_BYTECODE& data);
