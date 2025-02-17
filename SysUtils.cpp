#include "pch.h"
#include "SysUtils.h"
#include "string_conversion.h"
#include "resource.h"
#include <filesystem>

#if defined(OS_MACOS) || defined(OS_LINUX)
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <iconv.h>
#include <locale.h>
#include <sys/time.h>
#else
#include <clocale>
#endif

std::string SysUtils::get_full_path(const std::string& path)
{
    std::string path_folder = LoadStringResource(IDS_DRIVER_LOG_NAME);
    std::string full = path_folder + path;

    try {
        // Check if the directory exists
        std::filesystem::path dir(path_folder);
        if (!std::filesystem::exists(dir)) {
            // Create the directory and all subdirectories
            std::filesystem::create_directories(dir);
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem errors
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        full = "";
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
        full = "";
        // You can also log the error or take other appropriate actions
    }

    return full;
}
