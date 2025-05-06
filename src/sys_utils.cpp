#include "pch.h"
#include "sys_utils.h"
#include "string_conversion.h"
#include <filesystem>
#include "logger.h"

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

HMODULE SysUtils::g_hModule = nullptr;

std::wstring SysUtils::get_full_path(const std::wstring& path)
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

std::wstring SysUtils::getLogDriverFilePath() {
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
        SysUtils::LogException(L"Error create a directory for a log file", e);
    }

    return logPath;
}

void SysUtils::initLogging() {
 //   namespace logging = boost::log;
 //   namespace sinks = boost::log::sinks;
 //   namespace expr = boost::log::expressions;
 //   namespace attrs = boost::log::attributes;

 //   boost::shared_ptr<boost::log::core> core = boost::log::core::get();

 //   boost::shared_ptr<sinks::text_file_backend> backend = boost::make_shared<sinks::text_file_backend>(
 //       boost::log::keywords::file_name = "ecr_driver_%Y%m%d.log",
 //       boost::log::keywords::auto_flush = true
 //   );

 //   auto target_path = "f:\\tmp";
 //       //convertWStringToString(getLogDriverFilePath());
 //   auto _collector = boost::log::sinks::file::make_collector(
 //       boost::log::keywords::target = target_path,
 //       boost::log::keywords::min_free_space = 30 * 1024 * 1024
 //       
 //   );
	////_collector->store_file(target_path);
 //   backend->set_file_collector(_collector);
 //   backend->scan_for_files();

 //   typedef sinks::synchronous_sink<sinks::text_file_backend> file_sink;
 //   boost::shared_ptr<file_sink> sink(new file_sink(backend));

 //   sink->set_filter(logging::trivial::severity >= logging::trivial::debug);
 //   core->add_sink(sink);

 //   logging::add_common_attributes();
 //   core->add_global_attribute("Channel", attrs::constant<std::string>("Global"));
 //   core->add_global_attribute("TimeStamp", attrs::local_clock());
 //   core->add_global_attribute("ProcessID", attrs::current_process_id());
	//core->add_global_attribute("ThreadID", attrs::current_thread_id());
}

void SysUtils::LogInfo(const std::wstring& message) {
    //BOOST_LOG(gLibLog::get()) << logging::trivial::info << convertWStringToString(message);
}

void SysUtils::LogWarning(const std::wstring& message) {
    //BOOST_LOG(gLibLog::get()) << logging::trivial::warning << convertWStringToString(message);
}

void SysUtils::LogError(const std::wstring& message) {
    //BOOST_LOG(gLibLog::get()) << logging::trivial::error << convertWStringToString(message);
}

void SysUtils::LogException(const std::wstring& message, const std::exception& ex) {
    //BOOST_LOG(gLibLog::get()) << logging::trivial::error << convertWStringToString(message);
}

// Compare this snippet from SysUtils.cpp: