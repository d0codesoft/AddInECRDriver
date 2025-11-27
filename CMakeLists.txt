cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Detected MinGW toolchain with compiler: ${CMAKE_CXX_COMPILER_ID}")

        find_program(LLVM_LLD NAMES ld.lld lld)
        if(LLVM_LLD)
            add_link_options(-fuse-ld=lld)
            message(STATUS "Using LLD linker: ${LLVM_LLD}")
        else()
            message(WARNING "LLD не найден, ThinLTO может не работать. Установите mingw-w64-x86_64-lld.")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        endif()

        find_program(LLVM_AR NAMES llvm-ar)
        find_program(LLVM_RANLIB NAMES llvm-ranlib)
        if(LLVM_AR AND LLVM_RANLIB)
            set(CMAKE_AR "${LLVM_AR}" CACHE FILEPATH "Archiver" FORCE)
            set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
        else()
            find_program(GNU_AR NAMES ar)
            find_program(GNU_RANLIB NAMES ranlib)
            if(GNU_AR AND GNU_RANLIB)
                set(CMAKE_AR "${GNU_AR}" CACHE FILEPATH "Archiver" FORCE)
                set(CMAKE_RANLIB "${GNU_RANLIB}" CACHE FILEPATH "Ranlib" FORCE)
            else()
                message(FATAL_ERROR "Не удалось найти ar/ranlib. Установите llvm-ar/llvm-ranlib или binutils.")
            endif()
        endif()
    endif()
elseif(UNIX)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
endif()


project(POSTerminal VERSION 1.0.0 LANGUAGES CXX)

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

# Force CMake reconfigure when CMakeLists.txt changes
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake
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
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:inline>
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:throwingNew>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    )
    option(ENABLE_WERROR "Treat warnings as errors" OFF)
    if(ENABLE_WERROR)
        add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/WX>)
    endif()
else()
    # Generic warnings for GCC/Clang/AppleClang
    add_compile_options(
        # GCC (включая MinGW GCC)
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<CONFIG:Release>>>:-g>
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
        $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        $<$<CXX_COMPILER_ID:GNU>:-Wpedantic>
        $<$<CXX_COMPILER_ID:GNU>:-Wconversion>
        $<$<CXX_COMPILER_ID:GNU>:-Wshadow>

        # Clang
        $<$<CXX_COMPILER_ID:Clang>:-Wall>
        $<$<CXX_COMPILER_ID:Clang>:-Wextra>
        $<$<CXX_COMPILER_ID:Clang>:-Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wshadow>
        $<$<CXX_COMPILER_ID:Clang>:-Wconversion>

        # AppleClang
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wextra>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wpedantic>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wshadow>
    )
    option(ENABLE_WERROR "Treat warnings as errors" OFF)
    if(ENABLE_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

# Optional sanitizers (Debug / RelWithDebInfo non-MSVC)
option(ENABLE_SANITIZERS "Enable ASan/UBSan (GCC/Clang only)" OFF)

if (ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang") AND NOT MSVC)
    set(SAN_FLAGS "-fsanitize=address,undefined" "-fno-omit-frame-pointer")

    # Single-config generators (Makefile, Ninja)
    if (NOT CMAKE_CONFIGURATION_TYPES)
        if (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo")
            add_compile_options(${SAN_FLAGS})
            add_link_options(-fsanitize=address,undefined)
        endif()
    # Multi-config generators (VS, Ninja Multi-config)
    else()
        foreach(config Debug RelWithDebInfo)
            set_property(DIRECTORY APPEND PROPERTY
                COMPILE_OPTIONS_$<CONFIG:${config}> ${SAN_FLAGS}
            )
            set_property(DIRECTORY APPEND PROPERTY
                LINK_OPTIONS_$<CONFIG:${config}> -fsanitize=address,undefined
            )
        endforeach()
    endif()
endif()

# Optional IPO/LTO
option(ENABLE_IPO "Enable Interprocedural Optimization / LTO (Release only)" ON)
if(ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)
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
option(ENABLE_TEST_ADDIN "Build executable test_addin" ON)

# Sub-projects
add_subdirectory(src)

if(ENABLE_TEST_ADDIN)
    add_subdirectory(test_addin)
endif()