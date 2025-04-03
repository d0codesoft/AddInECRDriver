#include "pch.h"
#include "logger.h"
#include "string_conversion.h"
#include <mutex>
#include <filesystem>
#include <regex>


std::unordered_map<std::wstring, Logger*> Logger::InstancesLog;
std::mutex Logger::mutexLog;
std::filesystem::path Logger::_logDirectory;
std::wstring Logger::_logNameTemplate = log_name_default;
std::wofstream Logger::_LogFile;
size_t Logger::_maxFileSize;
size_t Logger::_maxFileCount;
bool Logger::isInitialized = false;
const std::filesystem::path Logger::current_log_path = std::filesystem::path(_logDirectory) / _logNameTemplate;

Logger::Logger(const std::wstring& channelName) : mChannelName(channelName) {
}

void Logger::removeInstance(const std::wstring& channelName)
{
    std::lock_guard<std::mutex> lock(mutexLog);
    Logger::InstancesLog.erase(channelName);
	if (Logger::InstancesLog.empty()) {
		if (_LogFile.is_open()) {
            _LogFile.close();
            isInitialized = false;
		}
	}
}

bool Logger::initializeFileStream()
{
    std::lock_guard<std::mutex> lock(mutexLog);

	isInitialized = false;
    if (_LogFile.is_open()) {
        _LogFile.close();
    }

	rotate_logs();

    //current_log_path = std::filesystem::path(_logDirectory) / _logNameTemplate;
    _LogFile.open(Logger::current_log_path, std::ios::app);
	if (_LogFile.is_open()) {
        isInitialized = true;
	}
	return isInitialized;
}

void Logger::rotate_logs()
{
	namespace fs = std::filesystem;

    std::vector<fs::path> log_files;
    std::wregex log_pattern(log_prefix + LR"(\.(\d+))");

	// create log directory if not exists
    if (!fs::exists(_logDirectory)) {
        fs::create_directory(_logDirectory);
    }

	// Test current log file size
    //fs::path current_log_path = fs::path(_logDirectory) / (log_prefix + L".log");
    if (!fs::exists(Logger::current_log_path) || (fs::exists(Logger::current_log_path) && fs::file_size(Logger::current_log_path) < _maxFileSize)) {
        return;
    }

	// Find all log files in the directory
    for (const auto& entry : fs::directory_iterator(_logDirectory)) {
        if (fs::is_regular_file(entry.path())) {
            if (std::regex_match(entry.path().filename().wstring(), log_pattern)) {
                log_files.push_back(entry.path());
            }
        }
    }

	// Sort log files by number
    std::sort(log_files.begin(), log_files.end(), [&](const fs::path& a, const fs::path& b) {
        std::wsmatch match_a, match_b;
        std::wstring filename_a = a.filename().wstring();
        std::regex_match(filename_a, match_a, log_pattern);
        std::regex_match(filename_a, match_b, log_pattern);
        return std::stoi(match_a[1]) < std::stoi(match_b[1]);
        });

	// If the number of log files exceeds the limit, delete the oldest
    while (log_files.size() > _maxFileCount) {
        fs::remove(log_files.front());
        log_files.erase(log_files.begin());
    }

	// Rename log files
    for (auto it = log_files.rbegin(); it != log_files.rend(); ++it) {
        std::wsmatch match;
        std::wstring filename_a = it->filename().wstring();
        std::regex_match(filename_a, match, log_pattern);
        int current_number = std::stoi(match[1]);
        fs::path new_path = it->parent_path() / (log_prefix + L"." + std::to_wstring(current_number + 1));
        fs::rename(*it, new_path);
    }

	// Rename current log file
    fs::rename(Logger::current_log_path, fs::path(_logDirectory) / (log_prefix + L".1"));
}

void Logger::logDebug(const std::wstring& level, const std::wstring& message, const std::wstring& file, int line)
{
	if (!isInitialized) {
		if (!initializeFileStream()) {
			return;
		}
	}

    std::lock_guard<std::mutex> lock(mutexLog);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::wstringstream ss;

    std::tm tm;
	// Test if the conversion was successful
    if (localtime_s(&tm, &now_c) == 0) { 
        ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S")
            << L" [" << mChannelName << L"] "
            << L"(" << file << L":" << line << L") "
            << level << L": " << message << std::endl;

        if (_LogFile.is_open()) {
            _LogFile << ss.str();
            _LogFile.flush();
        }
    }
}

void Logger::log(const std::wstring& level, const std::wstring& message)
{
    if (!isInitialized) {
        if (!initializeFileStream()) {
            return;
        }
    }

    std::lock_guard<std::mutex> lock(mutexLog);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::wstringstream ss;

    std::tm tm;
    if (localtime_s(&tm, &now_c) == 0) {
        ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S")
            << L" [" << mChannelName << L"] "
            << level << L": " << message << std::endl;

        if (_LogFile.is_open()) {
            _LogFile << ss.str();
            _LogFile.flush();
        }
    }
}

bool Logger::initialize(const std::wstring& logNameTemplate, const std::wstring& logDirectory, size_t maxFileSize, size_t maxFileCount)
{
    bool _initialized = false;

    _maxFileSize = maxFileSize;
    _maxFileCount = maxFileCount;

	if (_logNameTemplate == logNameTemplate && _logDirectory == logDirectory) {
		return true;
	}

    std::lock_guard<std::mutex> lock(mutexLog);
	if (!logDirectory.empty()) {
        _logDirectory = logDirectory;
    }
    if (!logNameTemplate.empty()) {
        _logNameTemplate = logNameTemplate;
    }
    
    if (!std::filesystem::exists(logDirectory)) {
        try {
            _initialized = std::filesystem::create_directories(logDirectory);
        }
        catch (const std::filesystem::filesystem_error& e) {
            _initialized = false;
        }
    }

	if (_initialized) {
        _initialized = initializeFileStream();
	}

    return _initialized;
}

Logger* Logger::getInstance(const std::wstring& channelName) {
    std::lock_guard<std::mutex> lock(mutexLog);
    auto it = Logger::InstancesLog.find(channelName);
    if (it == Logger::InstancesLog.end()) {
        auto logger = new Logger(channelName);
        Logger::InstancesLog[channelName] = logger;
        return logger;
    }
    return it->second;
}