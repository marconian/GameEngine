#pragma once

#include <filesystem>

const std::filesystem::path _getFontsDir();
std::filesystem::path GetFontsPath(std::string name, size_t size);
