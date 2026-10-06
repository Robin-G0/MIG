#include <mig/format/schema.hpp>

#include <nlohmann/json.hpp>

namespace mig::format {

std::string minimal_schema_descriptor() {
    const nlohmann::json descriptor = {
        {"format", "mig-motion"},
        {"version", 2},
    };
    return descriptor.dump();
}

} // namespace mig::format
