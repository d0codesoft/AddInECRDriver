#include "pch.h"
#include "logger.h"
#include "string_conversion.h"
#include <mutex>
#include <filesystem>
#include <regex>
#include "str_utils.h"
#include <sstream>
#include <iomanip>

#if defined(CURRENT_OS_WINDOWS)
#include <windows.h>
#include <shlobj.h>
#elif defined(CURRENT_OS_MACOS) || defined(CURRENT_OS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <iconv.h>
#include <locale.h>
#else
#include <clocale>
#endif

std::unordered_map<std::wstring, Logger*> Logger::InstancesLog;
std::mutex Logger::mutexLog;
std::filesystem::path Logger::_logDirectory;
std::wstring Logger::_logNameTemplate = log_name_default;
std::wofstream Logger::_LogFile;
size_t Logger::_maxFileSize;
size_t Logger::_maxFileCount;
bool Logger::isInitialized = false;
std::filesystem::path Logger::current_log_path = std::filesystem::path(_logDirectory) / _logNameTemplate;

Logger::Logger(std::wstring_view channelName) : mChannelName(channelName) {
}

std::wstring Logger::getLogFilePath()
{
	return Logger::current_log_path;
}

std::wstring Logger::get_full_path(const std::wstring& path)
{
    std::wstring path_folder = getLogDriverFilePath();
    std::wstring full = path_folder + path;

    try {
        std::filesystem::path dir(path_folder);
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return {};
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return {};
    }
    return full;
}

std::wstring Logger::getLogDriverFilePath()
{
    std::wstring logPath;

#if defined(_WIN32) || defined(_WIN64)
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
        logPath.assign(path);
        if (logPath.back() != L'\\') {
            logPath += L"\\";
        }
        logPath += L"AddInECRDriver\\logs";
    }
#elif defined(__linux__) || defined(__APPLE__)
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        homeDir = getpwuid(getuid())->pw_dir;
    }
    logPath = homeDir;
    logPath += "/.addinecrdriver/logs";
#endif

    try {
        std::filesystem::path dir(logPath);
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
		std::cout << "Filesystem error: " << e.what() << std::endl;
    }

    return logPath;
}

void Logger::removeInstance(std::wstring_view channelName)
{
    std::wstring channelStr(channelName);
    std::lock_guard<std::mutex> lock(mutexLog);
    Logger::InstancesLog.erase(channelStr);
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

    if (!fs::exists(Logger::current_log_path) ||
        (fs::exists(Logger::current_log_path) && fs::file_size(Logger::current_log_path) < _maxFileSize)) {
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
    std::sort(log_files.begin(), log_files.end(),
        [&](const fs::path& a, const fs::path& b) {
            std::wsmatch match_a, match_b;
            const std::wstring filename_a = a.filename().wstring();
            const std::wstring filename_b = b.filename().wstring();
            std::regex_match(filename_a, match_a, log_pattern);
            std::regex_match(filename_b, match_b, log_pattern);
            // Guard: if pattern fails, fallback to lexical compare
            if (match_a.size() < 2 || match_b.size() < 2) {
                return filename_a < filename_b;
            }
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
        const std::wstring filename = it->filename().wstring();
        if (std::regex_match(filename, match, log_pattern) && match.size() >= 2) {
            int current_number = std::stoi(match[1]);
            fs::path new_path = it->parent_path() / (log_prefix + L"." + std::to_wstring(current_number + 1));
            fs::rename(*it, new_path);
        }
    }

	// Rename current log file
    fs::rename(Logger::current_log_path, fs::path(_logDirectory) / (log_prefix + L".1"));
}

void Logger::logDebug(std::wstring_view level, std::wstring_view message, std::wstring_view file, int line)
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

void Logger::log(std::wstring_view level, std::wstring_view message)
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

bool Logger::initialize(std::wstring_view logNameTemplate, std::wstring_view logDirectory, size_t maxFileSize, size_t maxFileCount)
{
    bool _initialized = false;

    _maxFileSize = maxFileSize;
    _maxFileCount = maxFileCount;

	if (_logNameTemplate == logNameTemplate && _logDirectory == logDirectory) {
		return true;
	}

    //std::lock_guard<std::mutex> lock(mutexLog);
	if (!logDirectory.empty()) {
        _logDirectory = logDirectory;
        current_log_path = std::filesystem::path(_logDirectory) / _logNameTemplate;
    }
    if (!logNameTemplate.empty()) {
        _logNameTemplate = logNameTemplate;
    }
    
    if (!std::filesystem::exists(logDirectory)) {
        try {
            _initialized = std::filesystem::create_directories(logDirectory);
        }
        catch (const std::filesystem::filesystem_error& e) {
			throw std::runtime_error(e.what());
            _initialized = false;
        }
    }

	if (!_initialized) {
        _initialized = initializeFileStream();
	}

    return _initialized;
}

Logger* Logger::getInstance(std::wstring_view channelName) {
	std::wstring channelStr(channelName);
    std::lock_guard<std::mutex> lock(mutexLog);
    auto it = Logger::InstancesLog.find(channelStr);
    if (it == Logger::InstancesLog.end()) {
        auto logger = new Logger(channelStr);
        Logger::InstancesLog[channelStr] = logger;
        return logger;
    }
    return it->second;
}