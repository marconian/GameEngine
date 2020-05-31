#pragma once

#include <filesystem>

const std::filesystem::path _getFontsDir();
std::filesystem::path GetFontsPath(const std::string name, const size_t size);