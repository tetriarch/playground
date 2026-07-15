/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#include "tengine/utils/logger.hpp"

#include <chrono>

namespace tengine {

void assertHandler(
    const std::string& condition,
    const std::string& msg,
    const std::string& file,
    u32 line,
    const std::string& function
) {
    std::println("Assertion: '{}', \"{}\", {}:{} ^ {}", condition, msg, file, line, function);
}

Logger::~Logger() {
    for(auto&& s : sinks_) {
        s->flush();
    }
}

std::string logLevelToString(LogLevel logLevel, bool colored) {
    switch(logLevel) {
        case LogLevel::Trace: {
            return colored ? "\x1b[37;1mTRACE\x1b[0m" : "TRACE";
        }
        case LogLevel::Debug: {
            return colored ? "\x1b[34;1mDEBUG\x1b[0m" : "DEBUG";
        }
        case LogLevel::Info: {
            return colored ? "\x1b[32;1mINFO\x1b[0m" : "INFO";
        }
        case LogLevel::Warning: {
            return colored ? "\x1b[33;1mWARNING\x1b[0m" : "WARNING";
        }
        case LogLevel::Error: {
            return colored ? "\x1b[31;1mERROR\x1b[0m" : "ERROR";
        }
        case LogLevel::Fatal: {
            return colored ? "\x1b[30;1m\x1B[41mFATAL\x1b[0m" : "FATAL";
        }
        default: {
            return "UNKNOWN";
        }
    }
}

std::string dateTime() {
    auto tp = std::chrono::system_clock::now();
    return std::format("{:%d/%m/%Y %H:%M:%S}", tp);
}

void Logger::log(
    LogLevel logLevel,
    const std::string& system,
    const std::string& msg,
    const std::string& file,
    u32 line
) {
    for(auto&& s : sinks_) {
        s->log(logLevel, system, msg, file, line);
    }
}

void Logger::addSink(const std::shared_ptr<LogSink>& sink) {
    sinks_.emplace_back(sink);
}

void Logger::setLogLevel(LogLevel logLevel) {
    assert(!sinks_.empty() && "Logger error: call addSink() before setting log level!");
    for(auto&& s : sinks_) {
        s->setLogLevel(logLevel);
    }
}

void Logger::setColoredOutput(bool colored) {
    assert(!sinks_.empty() && "Logger error: call addSink() before setting color output!");
    for(auto&& s : sinks_) {
        s->setColoredOutput(colored);
    }
}

void Logger::flush() {
    for(auto&& s : sinks_) {
        s->flush();
    }
}

void ConsoleLogSink::setLogLevel(LogLevel logLevel) {
    logLevel_ = logLevel;
}

void ConsoleLogSink::setColoredOutput(bool colored) {
    if(!COLOR_SUPPORT) {
        coloredOutput_ = false;
        return;
    }
    coloredOutput_ = colored;
}

void ConsoleLogSink::log(
    LogLevel logLevel,
    const std::string& system,
    const std::string& msg,
    const std::string& file,
    u32 line
) {
    if(logLevel < logLevel_) {
        return;
    }

    std::string dt = dateTime();
    std::string lvl = logLevelToString(logLevel, coloredOutput_);

    std::lock_guard<std::mutex> guard(logMutex_);
    if(logLevel != LogLevel::Info) {
        std::println("[{}][{}]<{}>: {} || {}:{}", dt, lvl, system, msg, file, line);
        return;
    }
    std::println("[{}][{}]<{}>: {}", dt, lvl, system, msg);
}

FileLogSink::FileLogSink(const std::string& filePath) {
    file_.open(filePath, std::ios::out | std::ios::trunc);

    if(!file_.is_open()) {
        std::println("failed to open log file {}", filePath);
    }
}

FileLogSink::~FileLogSink() {
    if(file_.is_open()) {
        file_.close();
    }
}

void FileLogSink::setLogLevel(LogLevel logLevel) {
    logLevel_ = logLevel;
}

void FileLogSink::setColoredOutput(bool colored) {
    if(!COLOR_SUPPORT) {
        coloredOutput_ = false;
        return;
    }
    coloredOutput_ = colored;
}

void FileLogSink::flush() {
    if(file_.is_open()) {
        file_.flush();
    }
}

void FileLogSink::log(
    LogLevel logLevel,
    const std::string& system,
    const std::string& msg,
    const std::string& file,
    u32 line
) {
    if(logLevel < logLevel_) {
        return;
    }

    std::string dt = dateTime();
    std::string lvl = logLevelToString(logLevel, coloredOutput_);

    std::lock_guard<std::mutex> guard(logMutex_);
    std::string out;
    if(logLevel != LogLevel::Info) {
        out = std::format("[{}][{}] {}: {} || {}:{}\n", dt, lvl, system, msg, file, line);
    } else {
        std::println("[{}][{}] {}: {}", dt, lvl, system, msg);
    }
    if(file_.is_open()) {
        file_ << out;
    }
}

}  // namespace tengine
