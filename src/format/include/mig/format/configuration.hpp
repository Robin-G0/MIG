#pragma once
#include <filesystem>
#include <mig/core/engine.hpp>
namespace mig {
Configuration parse_configuration(std::string_view json);
std::string serialize_configuration(const Configuration& config);
Configuration load_configuration(const std::filesystem::path& path);
void save_configuration(const Configuration& config, const std::filesystem::path& path);
} // namespace mig
