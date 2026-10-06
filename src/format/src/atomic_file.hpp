#pragma once
#include <filesystem>
#include <string>

namespace mig::format::detail {
// Same-directory staging and atomic replacement; no shared predictable .tmp file.
void replace_document(const std::filesystem::path& destination, const std::string& document);
} // namespace mig::format::detail
