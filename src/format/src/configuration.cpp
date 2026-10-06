#include "atomic_file.hpp"
#include "json_model.hpp"
#include <fstream>
#include <mig/format/configuration.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>
namespace mig {
namespace {
constexpr std::size_t maximum_file_bytes = 1024 * 1024;
constexpr int maximum_json_depth = 32;

std::int64_t integer_field(const nlohmann::json& object, const char* name, std::int64_t minimum,
                           std::int64_t maximum) {
    const auto& value = object.at(name);
    if (!value.is_number_integer()) {
        throw std::runtime_error(std::string(name) + " must be an integer");
    }
    if (value.is_number_unsigned() && value.get<std::uint64_t>() > std::uint64_t(maximum)) {
        throw std::runtime_error(std::string(name) + " is out of range");
    }
    const auto result = value.get<std::int64_t>();
    if (result < minimum || result > maximum) {
        throw std::runtime_error(std::string(name) + " is out of range");
    }
    return result;
}

std::string read_document(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Cannot open configuration");
    }
    // Bound the actual bytes read, not a file_size check vulnerable to file growth.
    std::string bytes(maximum_file_bytes + 1, '\0');
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    const auto count = input.gcount();
    if (input.bad() || count > static_cast<std::streamsize>(maximum_file_bytes)) {
        throw std::runtime_error("Configuration read failed or exceeds 1 MiB");
    }
    bytes.resize(static_cast<std::size_t>(count));
    return bytes;
}
} // namespace
Configuration parse_configuration(std::string_view bytes) {
    if (bytes.size() > maximum_file_bytes) {
        throw std::runtime_error("Configuration exceeds 1 MiB");
    }
    const auto json =
        nlohmann::json::parse(bytes, [](int depth, nlohmann::json::parse_event_t, nlohmann::json&) {
            if (depth > maximum_json_depth) {
                throw std::runtime_error("Configuration nesting exceeds 32 levels");
            }
            return true;
        });
    integer_field(json, "schema_version", 2, 2);
    return format::detail::read_v2(json);
}
Configuration load_configuration(const std::filesystem::path& path) {
    return parse_configuration(read_document(path));
}
std::string serialize_configuration(const Configuration& config) {
    validate(config);
    const auto document = format::detail::write_v2(config).dump(4) + "\n";
    if (document.size() > maximum_file_bytes) {
        throw std::runtime_error("Saved configuration exceeds 1 MiB");
    }
    return document;
}
void save_configuration(const Configuration& config, const std::filesystem::path& path) {
    format::detail::replace_document(path, serialize_configuration(config));
}
} // namespace mig
