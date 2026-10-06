#pragma once
#include <mig/core/engine.hpp>
#include <nlohmann/json.hpp>
namespace mig::format::detail {
Configuration read_v2(const nlohmann::json& document);
nlohmann::json write_v2(const Configuration& config);
} // namespace mig::format::detail
