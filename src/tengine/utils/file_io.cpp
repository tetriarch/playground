/*
* ------
* Copyright (c) 2026 Petr Jirmus
* All rights reserved.
*/

#include "tengine/utils/file_io.hpp"

#include <fstream>

namespace tengine {
namespace file_io {

auto readBinaryFile(const std::filesystem::path& filePath)
    -> std::expected<std::vector<u8>, std::error_code> {
    if(!std::filesystem::exists(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    if(std::filesystem::is_directory(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::is_a_directory));
    }

    if(!std::filesystem::is_regular_file(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }

    std::ifstream file(filePath, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    std::vector<u8> content(std::istreambuf_iterator<char>(file), {});

    if(file.fail()) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    return content;
}

auto readTextFile(const std::filesystem::path& filePath)
    -> std::expected<std::string, std::error_code> {
    if(!std::filesystem::exists(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    if(std::filesystem::is_directory(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::is_a_directory));
    }

    if(!std::filesystem::is_regular_file(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }

    std::ifstream file(filePath, std::ios::in);

    if(!file.is_open()) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    std::string content, line;

    while(std::getline(file, line)) {
        content += line;
        if(!file.eof()) {
            content += '\n';
        }
    }

    return content;
}

auto writeBinaryFile(
    const std::filesystem::path& filePath, const std::vector<char>& content, bool append
) -> std::expected<void, std::error_code> {
    if(!std::filesystem::exists(filePath.parent_path())) {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    if(std::filesystem::is_directory(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::is_a_directory));
    }

    std::ios::openmode writeMode = (append == true) ? std::ios::app : std::ios::trunc;
    std::ofstream file(filePath, std::ios::out | std::ios::binary | writeMode);
    if(!file.is_open()) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    file.write(content.data(), content.size());

    return {};
}

auto writeTextFile(const std::filesystem::path& filePath, const std::string& content, bool append)
    -> std::expected<void, std::error_code> {
    if(!std::filesystem::exists(filePath.parent_path())) {
        return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
    }

    if(std::filesystem::is_directory(filePath)) {
        return std::unexpected(std::make_error_code(std::errc::is_a_directory));
    }

    std::ios::openmode writeMode = (append == true) ? std::ios::app : std::ios::trunc;
    std::ofstream file(filePath, std::ios::out | writeMode);
    if(!file.is_open()) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    file << content << '\n';

    return {};
}

}  // namespace file_io
}  // namespace tengine
