cmake_minimum_required(VERSION 3.21)

project(test_addin LANGUAGES CXX)

# C++20 requirement (PRIVATE for executable)
add_executable(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Sources
set(Header_Files
    AddInBaseStub.h
    common_platform.h
    ComponentBaseTester.h
    extension_test_script.h
    parser_call.h
    str_utils_tools.h
    wide_console.h
)
set(Source_Files
    AddInBaseStub.cpp
    common_platform.cpp
    ComponentBaseTester.cpp
    extension_test_script.cpp
    parser_call.cpp
    str_utils_tools.cpp
    test_addin.cpp
    wide_console.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${Header_Files} ${Source_Files})

# Optional MSVC props macro
if(MSVC AND COMMAND use_props)
    use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test")

# MSVC runtime
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/include/vncomp"
)

# Link to main library
if(TARGET ecr_driver_terminal)
    target_link_libraries(${PROJECT_NAME} PRIVATE ecr_driver_terminal)
else()
    message(FATAL_ERROR "ecr_driver_terminal target not found. Ensure src/CMakeLists.txt is processed first.")
endif()

# External dependencies
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

# Unix threading support
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

# Platform-specific system libs
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32)
endif()

# Compile definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE _UNICODE _CONSOLE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
endif()

# Compiler options (removed redundant -std=c++20)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4 
        /permissive- 
        /utf-8
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Gy>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:RelWithDebInfo>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
    )

    # Static linking for runtime libraries
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC: static libstdc++ and libgcc
        target_link_options(${PROJECT_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang with libstdc++
        if(NOT CMAKE_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+")
            target_link_options(${PROJECT_NAME} PRIVATE
                -static-libgcc
                -static-libstdc++
            )
        else()
            # Clang with libc++
            target_link_options(${PROJECT_NAME} PRIVATE
                -static
            )
        endif()
    endif()
endif()

# Optional IPO/LTO
option(TEST_ADDIN_ENABLE_IPO "Enable LTO/IPO for test_addin Release" ON)
if(TEST_ADDIN_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
    if(ipo_supported)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    else()
        message(STATUS "IPO not supported for test_addin: ${ipo_msg}")
    endif()
endif()

# Output directories (multi-config and single-config)
set(output_root "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}")
if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${cfg}" cfgU)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${cfgU} "${output_root}/${cfg}"
            ARCHIVE_OUTPUT_DIRECTORY_${cfgU} "${output_root}/${cfg}"
            LIBRARY_OUTPUT_DIRECTORY_${cfgU} "${output_root}/${cfg}"
        )
    endforeach()
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
    )
endif()

# Optionally enable sanitizers (non-MSVC debug)
option(TEST_ADDIN_SANITIZERS "Enable address/undefined sanitizers for test_addin" OFF)
if(TEST_ADDIN_SANITIZERS AND NOT MSVC AND (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo"))
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
endif()