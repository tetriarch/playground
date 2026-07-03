/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#pragma once

#include <fstream>
#include <mutex>

#include "tengine/aliases.hpp"
#include "tengine/globals.hpp"

constexpr std::string DEFAULT_LOG_PATH = "log.txt";

#define TENGINE_ASSERT(cond, msg, ...)                                                            \
    (static_cast<bool>(cond) ? void(0)                                                            \
                             : (tengine::assertHandler(                                           \
                                    #cond, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, \
                                    __LINE__, __FUNCTION__                                        \
                                ),                                                                \
                                __builtin_trap()))

#define LOGGER tengine::logger()

#define LOG_TRACE(system, msg, ...)                                                              \
    LOGGER->log(                                                                                 \
        LogLevel::Trace, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );

#define LOG_DEBUG(system, msg, ...)                                                              \
    LOGGER->log(                                                                                 \
        LogLevel::Debug, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );

#define LOG_INFO(system, msg, ...)                                                              \
    LOGGER->log(                                                                                \
        LogLevel::Info, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );

#define LOG_WARN(system, msg, ...)                                                                 \
    LOGGER->log(                                                                                   \
        LogLevel::Warning, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );

#define LOG_ERROR(system, msg, ...)                                                              \
    LOGGER->log(                                                                                 \
        LogLevel::Error, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );

#define LOG_FATAL(system, msg, ...)                                                              \
    LOGGER->log(                                                                                 \
        LogLevel::Fatal, system, std::format(msg __VA_OPT__(, ) __VA_ARGS__), __FILE__, __LINE__ \
    );                                                                                           \
    LOGGER->flush();                                                                             \
    __builtin_trap();

enum class LogLevel : u8 {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

namespace tengine {

class LogSink;
class ConsoleLogSink;
class FileLogSink;

void assertHandler(
    const std::string& condition,
    const std::string& msg,
    const std::string& file,
    u32 line,
    const std::string& function
);

class Logger {
public:
    Logger() = default;
    ~Logger();

public:
    void log(
        LogLevel logLevel,
        const std::string& system,
        const std::string& msg,
        const std::string& file,
        u32 line
    );

    void addSink(const std::shared_ptr<LogSink>& sink);
    void setLogLevel(LogLevel logLevel);
    void setColoredOutput(bool colored);
    void flush();

private:
    std::vector<std::shared_ptr<LogSink>> sinks_;

private:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(const Logger&&) = delete;
    Logger& operator=(const Logger&&) = delete;
};

class LogSink {
public:
    virtual ~LogSink() = 0;
    virtual void setLogLevel(LogLevel logLevel) = 0;
    virtual void setColoredOutput(bool colored) = 0;
    virtual void flush() = 0;
    virtual void log(
        LogLevel logLevel,
        const std::string& system,
        const std::string& msg,
        const std::string& file,
        u32 line
    ) = 0;

private:
    friend ConsoleLogSink;
    friend FileLogSink;

    LogSink() = default;
    LogLevel logLevel_ = LogLevel::Trace;
    bool coloredOutput_ = false;
    std::mutex logMutex_;
};

inline LogSink::~LogSink() {
}

class ConsoleLogSink final : public LogSink {
public:
    void log(
        LogLevel logLevel,
        const std::string& system,
        const std::string& msg,
        const std::string& file,
        u32 line
    ) override;

    void setLogLevel(LogLevel logLevel) override;
    void setColoredOutput(bool colored) override;
    void flush() override {};

private:
    const bool COLOR_SUPPORT = true;
};

class FileLogSink final : public LogSink {
public:
    FileLogSink(const std::string& filePath = DEFAULT_LOG_PATH);
    ~FileLogSink();

public:
    void log(
        LogLevel logLevel,
        const std::string& system,
        const std::string& msg,
        const std::string& file,
        u32 line
    ) override;

    void setLogLevel(LogLevel logLevel) override;
    void setColoredOutput(bool colored) override;
    void flush() override;

private:
    std::ofstream file_;
    const bool COLOR_SUPPORT = false;
};

}  // namespace tengine
