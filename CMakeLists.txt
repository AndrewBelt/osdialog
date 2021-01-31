cmake_minimum_required(VERSION 3.16)


# Add the properties of a pkg-config package to a target
macro(target_add_package target package)
    target_include_directories(${target} PRIVATE ${${package}_INCLUDE_DIRS})
    target_link_directories(${target} PRIVATE ${${package}_LIBRARY_DIRS})

    target_compile_options(${target} PRIVATE ${${package}_CFLAGS_OTHER})
    target_link_options(${target} PRIVATE ${${package}_LDFLAGS_OTHER})

    target_link_libraries(${target} PRIVATE ${${package}_LIBRARIES})
endmacro()


# Set root directory, if needed
if (NOT OSDLG_ROOT_DIR)
    set(OSDLG_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()


# Set the name of the project
set(target_name osdialog)
project(${target_name} C)


# Use C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Check and set command-line arguments
if (UNIX AND NOT APPLE)
    if (NOT "${OSDLG_TOOLKIT}" STREQUAL "GTK" AND
        NOT "${OSDLG_TOOLKIT}" STREQUAL "GTK3" AND
        NOT "${OSDLG_TOOLKIT}" STREQUAL "ZENITY")
        set(OSDLG_TOOLKIT "GTK3")
        message("--== Setting default toolkit ${OSDLG_TOOLKIT}")
    endif()
    message("--== Using toolkit ${OSDLG_TOOLKIT}")
endif()

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared lib")


# Find required packages
if (APPLE)
    find_library(APPKIT_PATH AppKit)
elseif (UNIX AND NOT APPLE)
    if ("${OSDLG_TOOLKIT}" STREQUAL "GTK3")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    elseif ("${OSDLG_TOOLKIT}" STREQUAL "GTK")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK REQUIRED gtk+-2.0)
    elseif ("${OSDLG_TOOLKIT}" STREQUAL "ZENITY")
        find_program(ZENITY_PATH zenity REQUIRED)
    endif()
endif()


# Gather the source files
if (MSVC)
    set(OSDLG_SOURCES src/osdialog_win.c)
elseif (APPLE)
    set(OSDLG_SOURCES src/osdialog_mac.m)
elseif (UNIX AND NOT APPLE)
    if ("${OSDLG_TOOLKIT}" STREQUAL "GTK3")
        set(OSDLG_SOURCES src/osdialog_gtk3.c)
    elseif ("${OSDLG_TOOLKIT}" STREQUAL "GTK")
        set(OSDLG_SOURCES src/osdialog_gtk2.c)
    elseif ("${OSDLG_TOOLKIT}" STREQUAL "ZENITY")
        set(OSDLG_SOURCES src/osdialog_zenity.c)
    endif()
endif()
set(OSDLG_SOURCES ${OSDLG_SOURCES} src/osdialog.c)


# Gather the header files
set(OSDLG_HEADERS_PUBLIC inc/osdialog.h)


# Gather the include directories
set(OSDLG_INCLUDE_DIRS_PUBLIC inc)


# Add the target
add_library(${target_name}
    ${OSDLG_SOURCES}
    ${OSDLG_HEADERS_PUBLIC})


# Add package properties
if (UNIX AND NOT APPLE)
    target_add_package(${target_name} ${OSDLG_TOOLKIT})
endif()


# Link third-party libraries
if (MSVC)
    target_link_libraries(${target_name} PRIVATE comdlg32)
elseif (APPLE)
    target_link_libraries(${target_name} PRIVATE ${APPKIT_PATH})
endif()


# Set target properties
target_include_directories(${target_name} PUBLIC ${OSDLG_INCLUDE_DIRS_PUBLIC})


# Set source file properties
set_source_files_properties(${OSDLG_HEADERS_PUBLIC} PROPERTIES HEADER_FILE_ONLY TRUE)


# Group files under logical folders (mainly for IDEs)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES
    ${OSDLG_SOURCES}
    ${OSDLG_HEADERS_PUBLIC})


# Add definitions (specific to this project)
if (MSVC)
    set(OSDLG_CXX_DEFS PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_definitions(${target_name} PRIVATE ${OSDLG_CXX_DEFS})


# Add compile options (specific to this project)
if (MSVC)
    set(OSDLG_CXX_FLAGS /W4 /WX-)
else()
    set(OSDLG_CXX_FLAGS -Wall -Wextra -pedantic)
    # TODO(developer): fix these
    set(OSDLG_CXX_FLAGS ${OSDLG_CXX_FLAGS}
        -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-parameter)
endif()
target_compile_options(${target_name} PRIVATE ${OSDLG_CXX_FLAGS})


# Build tests
if (OSDLG_BUILD_TESTS)
    set(OSDLG_TEST_SOURCES tests/test.c)

    add_executable(${target_name}_test
        ${OSDLG_TEST_SOURCES})

    target_link_libraries(${target_name}_test PRIVATE ${target_name})

    # Group files under logical folders (mainly for IDEs)
    source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES
        ${OSDLG_TEST_SOURCES})

endif()
