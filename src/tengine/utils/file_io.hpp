/*
* ------
* Copyright (c) 2026 Petr Jirmus
* All rights reserved.
*/

#pragma once

#include "tengine/aliases.hpp"

namespace tengine {
namespace file_io {

[[nodiscard]] auto readBinaryFile(const std::filesystem::path& filePath)
    -> std::expected<std::vector<u8>, std::error_code>;

[[nodiscard]] auto readTextFile(const std::filesystem::path& filePath)
    -> std::expected<std::string, std::error_code>;

auto writeBinaryFile(
    const std::filesystem::path& filePath, const std::vector<char>& content, bool append = false
) -> std::expected<void, std::error_code>;

auto writeTextFile(
    const std::filesystem::path& filePath, const std::string& content, bool append = false
) -> std::expected<void, std::error_code>;

}  // namespace file_io
}  // namespace tengine
