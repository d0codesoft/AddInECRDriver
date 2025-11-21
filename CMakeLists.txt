cmake_minimum_required(VERSION 3.21 FATAL_ERROR) # 3.21+ for MSVC runtime policy & modern features

# Optional: set project version
project(POSTerminal
    VERSION 1.0.0
    LANGUAGES CXX
)

message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")

# Optionally allow embedding as subproject (disable strict guard)
option(STRICT_TOPLEVEL_GUARD "Fail if included from another project" OFF)
if(STRICT_TOPLEVEL_GUARD)
    if(NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        message(FATAL_ERROR "Do not include this top-level CMakeLists.txt from another project.")
    endif()
endif()

# Prefer explicit C++ standard (avoid relying on target consumers for now)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (clang-tidy, include-graph, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type for single-config generators (avoid FORCE if user set it)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug|Release|RelWithDebInfo|MinSizeRel)" )
endif()

# Multi-config set (Visual Studio, Ninja Multi-Config)
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Multi-Config")
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)
endif()

# Architecture validation (MSVC multi-config generator)
if(CMAKE_VS_PLATFORM_NAME)
    message(STATUS "${CMAKE_VS_PLATFORM_NAME} architecture in use")
    if(NOT CMAKE_VS_PLATFORM_NAME MATCHES "^(x64|x86)$")
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
else()
    # Fallback for single-config or non-MSVC scenarios
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_ARCH x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_ARCH x86)
    else()
        set(_ARCH unknown)
    endif()
    message(STATUS "Detected architecture: ${_ARCH}")
endif()

# MSVC specific setup
if(MSVC)
    if(POLICY CMP0091)
        cmake_policy(SET CMP0091 NEW)
    endif()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /Zc:inline /Zc:throwingNew /utf-8>
    )
    option(ENABLE_WERROR "Treat warnings as errors" OFF)
    if(ENABLE_WERROR)
        add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/WX>)
    endif()
else()
    # Generic warnings for GCC/Clang/AppleClang
    add_compile_options(
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<CONFIG:Release>>>:-g>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wconversion -Wshadow>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Wpedantic -Wshadow>
    )
    option(ENABLE_WERROR "Treat warnings as errors" OFF)
    if(ENABLE_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

# Optional sanitizers (Debug / RelWithDebInfo non-MSVC)
option(ENABLE_SANITIZERS "Enable address/undefined sanitizers (non-MSVC)" OFF)
if(ENABLE_SANITIZERS AND NOT MSVC AND (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo"))
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Optional IPO/LTO
option(ENABLE_IPO "Enable Interprocedural Optimization / LTO (Release only)" ON)
if(ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    else()
        message(STATUS "IPO not supported: ${ipo_msg}")
    endif()
endif()

# NuGet package stub
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "use_package() not implemented. PACKAGE='${PACKAGE}' VERSION='${VERSION}' TARGET='${TARGET}' ignored.")
endfunction()

# Utilities & optional global settings
include(CMake/Utils.cmake)
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

# Organize targets into folders (Visual Studio)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Feature toggles
option(ENABLE_TEST_ADDIN "Build test_addin subproject" OFF)

# Sub-projects
add_subdirectory(src)

if(ENABLE_TEST_ADDIN)
    add_subdirectory(test_addin)
endif()