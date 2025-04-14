#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <memory>

const std::wstring log_prefix = L"ecr_driver";
const std::wstring log_extension = L".log";
const std::wstring log_name_default = log_prefix + log_extension;



class Logger
{
public:

    static bool initialize(const std::wstring& logNameTemplate, const std::wstring& logDirectory, size_t maxFileSize, size_t maxFileCount);

    static Logger* getInstance(const std::wstring& channelName);

    explicit Logger(const std::wstring& channelName);
	~Logger() {
		removeInstance(mChannelName);
    }

	static std::wstring getLogFilePath();

#ifdef NDEBUG
    void info(const std::wstring& message, const std::wstring& file = L"", int line = 0) {
        log(L"INFO", message, file, line);
    }

    void warn(const std::wstring& message, const std::wstring& file = L"", int line = 0) {
        log(L"WARN", message, file, line);
    }

    void error(const std::wstring& message, const std::wstring& file = L"", int line = 0) {
        log(L"ERROR", message, file, line);
    }
#else
    void info(const std::wstring& message) {
        log(L"INFO", message);
    }

    void warn(const std::wstring& message) {
        log(L"WARN", message);
    }

    void error(const std::wstring& message) {
        log(L"ERROR", message);
    }
#endif

private:
    static void removeInstance(const std::wstring& channelName);
    static bool initializeFileStream();
    static void rotate_logs();

    void logDebug(const std::wstring& level, const std::wstring& message, const std::wstring& file, int line);
    void log(const std::wstring& level, const std::wstring& message);

    std::wstring mChannelName;

    static std::unordered_map<std::wstring, Logger*> InstancesLog;
    static std::mutex mutexLog;
    static std::filesystem::path _logDirectory;
    static std::wstring _logNameTemplate;
    static size_t _maxFileSize;
    static size_t _maxFileCount;
    static std::wofstream _LogFile;
	static bool isInitialized;
	static const std::filesystem::path current_log_path;
};

#ifdef NDEBUG
#define LOG_INFO(logger, message) logger->info(message, __FILE__, __LINE__)
#define LOG_WARN(logger, message) logger->warn(message, __FILE__, __LINE__)
#define LOG_ERROR(logger, message) logger->error(message, __FILE__, __LINE__)
#else
#define LOG_INFO(logger, message) logger->info(message)
#define LOG_WARN(logger, message) logger->warn(message)
#define LOG_ERROR(logger, message) logger->error(message)
#endif

// Macro to initialize the logger
#define LOGGER_INITIALIZE(logNameTemplate, logDirectory, maxFileSize, maxFileCount) \
    Logger::initialize(logNameTemplate, logDirectory, maxFileSize, maxFileCount)

// Macro to initialize the logger
#define LOGGER_INITIALIZE_DEFAULT(logDirectory) \
    Logger::initialize(log_name_default, logDirectory, 10 * 1024 * 1024, 5)

// Macros to get logger instance and write log messages
#define LOG_INFO_ADD(channelName, message) \
    Logger::getInstance(channelName)->info(message)

#define LOG_WARN_ADD(channelName, message) \
    Logger::getInstance(channelName)->warn(message)

#define LOG_ERROR_ADD(channelName, message) \
    Logger::getInstance(channelName)->error(message)

#define LOG_ERROR_ADD_EX(channelName, message, exception) \
    Logger::getInstance(channelName)->error(message + L" Exception: " + convertStringToWString(exception.what()))

#endif // LOGGER_H


// Пример использования:
// auto& networkLogger = Logger::getInstance("Network");
// LOG_INFO(networkLogger, "Подключение установлено к серверу {}", "192.168.1.1");
// auto& fileLogger = Logger::getInstance("File");
// LOG_ERROR(fileLogger, "Ошибка записи файла {}", "output.txt");
