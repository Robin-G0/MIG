#include <mig/core/library.hpp>

namespace mig::core {

std::string_view library_version() noexcept {
    return MIG_LIBRARY_VERSION;
}

} // namespace mig::core
