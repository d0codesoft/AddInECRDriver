# Cross‑platform CMakeLists.txt for ecr_driver_terminal
# Converted from a Windows/MSVC centric configuration to a portable one.
# Retains MSVC-specific optimizations while enabling Linux/macOS builds.

# Library name (avoid overriding parent PROJECT_NAME)
set(lib_name ecr_driver_terminal)

# Collect sources (avoid semicolons inside quoted arguments; treat paths cleanly)
set(Header_Files
    addin_driver.h
    common_main.h
    common_types.h
    connection_types.h
    define_driver.h
    driver_pos_terminal.h
    framework.h
    interface_addin_base.h
    interface_connection.h
    interface_driver_base.h
    interface_pos_terminal.h
    license_manager.h
    localization_consts.h
    localization_manager.h
    logger.h
    manager_hwid.h
    pch.h
    protocol_consts.h
    setting_driver_pos.h
    storage_manager.h
    str_utils.h
    string_conversion.h
    sys_utils.h
    tvariant_helper.h
    сhannel_protocol.h # WARNING: leading 'с' is Cyrillic; consider renaming to 'c'
)
set(Source_Files
    addin_driver.cpp
    common_main.cpp
    common_types.cpp
    connection_types.cpp
    dllmain.cpp
    driver_pos_terminal.cpp
    interface_connection.cpp
    interface_driver_base.cpp
    license_manager.cpp
    localization_consts.cpp
    localization_manager.cpp
    logger.cpp
    manager_hwid.cpp
    pch.cpp
    protocol_consts.cpp
    setting_driver_pos.cpp
    storage_manager.cpp
    str_utils.cpp
    string_conversion.cpp
    sys_utils.cpp
    tvariant_helper.cpp
    сhannel_protocol.cpp
)
source_group("Header Files" FILES ${Header_Files})
source_group("Source Files" FILES ${Source_Files})

add_library(${lib_name} SHARED ${Header_Files} ${Source_Files})

# C++20 (portable, not global)
target_compile_features(${lib_name} PUBLIC cxx_std_20)

# Precompiled header (supported by MSVC, Clang, GCC since CMake 3.16)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${lib_name} PRIVATE pch.h)
endif()

# Keep custom property propagation only if provided (Windows-specific)
if(MSVC AND COMMAND use_props)
    use_props(${lib_name} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
endif()

set_target_properties(${lib_name} PROPERTIES
    FOLDER "ecr"
    # Config-specific output naming (Debug vs Release)
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,ECR_Driver_PT_dbg,ECR_Driver_PT>"
)

# Architecture suffix for Windows builds, if you still need distinct names
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set_property(TARGET ${lib_name} PROPERTY OUTPUT_NAME "${lib_name}_win64")
	else()
		set_property(TARGET ${lib_name} PROPERTY OUTPUT_NAME "${lib_name}_win32")
	endif()
endif()

# Position independent code (PIC) for shared libs on Unix
if(UNIX AND NOT APPLE)
    set_target_properties(${lib_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# MSVC runtime (only if parent set CMP0091 NEW)
if(MSVC)
    set_property(TARGET ${lib_name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Include directories (PUBLIC if headers used by consumers)
target_include_directories(${lib_name} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/pugixml"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/vncomp"
)

# --- Boost auto detection & usage ---
option(USE_SYSTEM_BOOST "Use system-installed Boost via CONFIG mode" ON)

if(USE_SYSTEM_BOOST)

    # Required Boost components
    find_package(boost 1.89 CONFIG REQUIRED)

    # Link libraries
    target_link_libraries(${lib_name} PUBLIC
        Boost::boost
    )

    # MSVC runtime customization (optional)
    if(MSVC)
        set_property(TARGET ${lib_name} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
else()
    # Fallback: manual local Boost (header-only + staged libs)
    target_include_directories(${lib_name} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../boost"
    )
    if(WIN32)
        target_link_directories(${lib_name} PRIVATE
            "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../../boost/stage/lib>"
        )
    endif()
endif()

# OpenSSL (cross-platform)
find_package(OpenSSL REQUIRED)
target_link_libraries(${lib_name} PUBLIC OpenSSL::SSL OpenSSL::Crypto)

# Platform-specific system libraries
if(WIN32)
    target_link_libraries(${lib_name} PRIVATE
        ws2_32
        crypt32
    )
    target_compile_definitions(${lib_name} PRIVATE
        UNICODE _UNICODE
        ECRDRIVER_EXPORTS
        _WINDOWS
        _USRDLL
        $<$<CONFIG:Debug>:_DEBUG>
		$<$<CONFIG:Release>:NDEBUG>
    )
elseif(APPLE)
    # macOS frameworks if needed (placeholder)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    if(COREFOUNDATION_FRAMEWORK)
        target_link_libraries(${lib_name} PRIVATE "${COREFOUNDATION_FRAMEWORK}")
    endif()
    target_compile_definitions(${lib_name} PRIVATE
        ECRDRIVER_EXPORTS
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
elseif(UNIX)
    # pthread, dl, rt as needed
    find_package(Threads REQUIRED)
    target_link_libraries(${lib_name} PRIVATE Threads::Threads)
    # Check existence before linking (dl, rt sometimes optional)
    find_library(DL_LIBRARY dl)
    if(DL_LIBRARY) 
		target_link_libraries(${lib_name} PRIVATE "${DL_LIBRARY}") 
	endif()
    find_library(RT_LIBRARY rt)
    if(RT_LIBRARY) 
		target_link_libraries(${lib_name} PRIVATE "${RT_LIBRARY}") 
	endif()
    target_compile_definitions(${lib_name} PRIVATE
        ECRDRIVER_EXPORTS
        _GNU_SOURCE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
endif()

# Warning level & common compile options
if (MSVC)
    # Компиляция
    target_compile_options(${lib_name} PRIVATE
        /W4            # повышенный warning level (VS сам даёт /W3)
        /permissive-   # строгий стандарт C++
        /Zc:inline      # стандартное поведение inline
        /Zc:preprocessor
        /Zc:forScope
        /utf-8
        /std:c++20
        /FC            # показать полный путь к файлам в ошибках
        /diagnostics:column
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od /RTC1>
    )

    # Параметры исключений / runtime
    target_compile_options(${lib_name} PRIVATE /EHsc)

    # Runtime library
    set_property(TARGET ${lib_name} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

    # Линковка
    target_link_options(${lib_name} PRIVATE
        /DLL
        /MANIFEST
        /MANIFESTUAC:NO
        /INCREMENTAL
        /SUBSYSTEM:WINDOWS
        /MACHINE:X64
        /ERRORREPORT:PROMPT
        $<$<CONFIG:Release>:/OPT:REF /OPT:ICF>
    )
else()
	target_compile_options(${lib_name} PRIVATE
		# Базовые предупреждения
		-Wall
		-Wextra
		-Wpedantic

		# Дополнительные полезные
		-Wconversion
		-Wsign-conversion
		-Wshadow
		-Wformat=2
		-Wnull-dereference
		-Wdouble-promotion
		-Woverloaded-virtual
		-Wimplicit-fallthrough
		-Wunused
		-Wuninitialized

		# PIC для shared библиотек на Linux/macOS
		-fPIC

		# C++20 — для GCC/Clang это безопасно
		-std=c++20

		# Контроль UB
		$<$<AND:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<CONFIG:Debug>>:-fsanitize=undefined>
		$<$<AND:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<CONFIG:Debug>>:-fsanitize=address>
		$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang>,$<CONFIG:Debug>>:-fsanitize=undefined>
		$<$<AND:$<COMPILE_LANG_AND_ID:CXX,Clang>,$<CONFIG:Debug>>:-fsanitize=address>

		# RTTI и исключения
		-frtti
		-fexceptions

		# Режимы конфигурации
		$<$<CONFIG:Release>:-O3 -DNDEBUG>
		$<$<CONFIG:RelWithDebInfo>:-O2 -g>
		$<$<CONFIG:Debug>:-O0 -g -DDEBUG>
	)
endif()

# Interprocedural optimization / LTO (guarded by compiler support)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
if(ipo_supported)
    set_property(TARGET ${lib_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# Output directories (use generic variables; avoid VS-only properties)
set(output_root "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}")
foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${cfg}" cfg_upper)
    set_target_properties(${lib_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} "${output_root}/${cfg}"
        LIBRARY_OUTPUT_DIRECTORY_${cfg_upper} "${output_root}/${cfg}"
        ARCHIVE_OUTPUT_DIRECTORY_${cfg_upper} "${output_root}/${cfg}"
    )
endforeach()

# Single-config generator fallback
if(NOT CMAKE_CONFIGURATION_TYPES)
    set_target_properties(${lib_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_root}/${CMAKE_BUILD_TYPE}"
    )
endif()

# Post-build Python packaging (cross-platform path & gated on existence)
option(ENABLE_DRIVER_PACKAGING "Run driver packaging script after build" ON)
if(ENABLE_DRIVER_PACKAGING)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_custom_command(TARGET ${lib_name} POST_BUILD
            COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_SOURCE_DIR}/tools/make_driver.py"
                -build_type "$<CONFIG>"
                -build_dir "${output_root}/$<CONFIG>"
                -output_dir "${CMAKE_SOURCE_DIR}/tools"
            COMMENT "Packaging ecr_driver_terminal for 1C AddIn"
            VERBATIM
        )
    else()
        message(WARNING "Python3 interpreter not found; packaging skipped.")
    endif()
endif()

# Optional: export an interface for consumers
add_library(ecr_driver_terminal::ecr ALIAS ${lib_name})