# SPDX-License-Identifier: MIT
# Copyright (c) 2026 MTL
cmake_minimum_required(VERSION 3.16)
project(OpenKeyboard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ------------------------------------------------------------------
# Third-party subproject friendliness:
# When included via add_subdirectory (as a dependency, e.g. TCU), default
# example/install OFF so the host isn't forced to build them. Standalone
# builds keep the full example + install experience.
# ------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(OPENKEYBOARD_DEFAULT_EXAMPLE ON)
    set(OPENKEYBOARD_DEFAULT_INSTALL ON)
else()
    set(OPENKEYBOARD_DEFAULT_EXAMPLE OFF)
    set(OPENKEYBOARD_DEFAULT_INSTALL OFF)
endif()

option(OPENKEYBOARD_BUILD_EXAMPLE "Build example app" ${OPENKEYBOARD_DEFAULT_EXAMPLE})
option(OPENKEYBOARD_INSTALL "Generate install rules" ${OPENKEYBOARD_DEFAULT_INSTALL})
option(OPENKEYBOARD_BUILD_PLATFORM_PLUGIN "Build QPlatformInputContext plugin (QT_IM_MODULE=openkeyboard)" ON)

# Public variables host projects can read after add_subdirectory():
#   OpenKeyboard_SOURCE_DIR        - absolute source root
#   OpenKeyboard_QML_IMPORT_PATH   - dir containing qmldir (import OpenKeyboard 1.0)
#   OpenKeyboard_QML_QRC           - absolute path to qml/openkeyboard.qrc (nest in host .qrc)
#   OpenKeyboard_PLUGINS_DIR       - build-tree plugin root (QT_PLUGIN_PATH)
# Use CACHE so the values propagate to the host project scope after add_subdirectory().
set(OpenKeyboard_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "OpenKeyboard source root" FORCE)
set(OpenKeyboard_QML_IMPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH
    "Directory to pass to engine.addImportPath() so 'import OpenKeyboard 1.0' resolves (module root is <this>/OpenKeyboard)" FORCE)
set(OpenKeyboard_QML_QRC "${CMAKE_CURRENT_SOURCE_DIR}/qml/openkeyboard.qrc" CACHE FILEPATH
    "Absolute path to qml/openkeyboard.qrc (nest inside host .qrc)" FORCE)
set(OpenKeyboard_PLUGINS_DIR "${CMAKE_BINARY_DIR}/plugins" CACHE PATH
    "Directory under which the platform input context plugin is built (QT_PLUGIN_PATH root)")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Gui)

set(OPENKEYBOARD_QML_FILES
    qml/OpenInputPanel.qml
    qml/OpenKeyboard.qml
    qml/InputEngine.qml
    qml/CandidateBar.qml
    qml/styles/KeyboardStyle.qml
    qml/styles/compact/Style.qml
    qml/styles/default/Style.qml
    qml/components/KeyButton.qml
    qml/components/TextKey.qml
    qml/components/FunctionKey.qml
    qml/components/ActionKey.qml
    qml/layouts/QwertyLayout.qml
    qml/layouts/SymbolsLayout.qml
    qml/layouts/NumberLayout.qml
)

if(OPENKEYBOARD_BUILD_PLATFORM_PLUGIN)
    add_compile_definitions(OPENKEYBOARD_HAS_PLATFORM_PLUGIN=1)
endif()

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

# QML install dir (used by install rules and package config)
set(CMAKE_INSTALL_QMLDIR "${CMAKE_INSTALL_DATADIR}/qml/OpenKeyboard" CACHE STRING "QML install dir")

# ------------------------------------------------------------------
# Core library: third-party dynamic library (SHARED)
# ------------------------------------------------------------------
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    add_library(openkeyboard SHARED
        src/bridge/openkeyboardbridge.cpp
        src/bridge/openkeyboardbridge.h
    )
    # Attach QML module to the same SHARED target (Qt >= 6.2)
    qt_add_qml_module(openkeyboard
        URI OpenKeyboard
        VERSION 1.0
        QML_FILES ${OPENKEYBOARD_QML_FILES}
        RESOURCE_PREFIX "/qt/qml"
    )
    target_link_libraries(openkeyboard PUBLIC Qt6::Core Qt6::Quick Qt6::Gui)
else()
    add_library(openkeyboard SHARED
        src/bridge/openkeyboardbridge.cpp
        src/bridge/openkeyboardbridge.h
    )
    target_link_libraries(openkeyboard PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
endif()

target_include_directories(openkeyboard PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/bridge>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# Define export macro for building the library itself
target_compile_definitions(openkeyboard PRIVATE OPENKEYBOARD_BUILD)
# Propagate platform-plugin availability to consumers (host code can
# conditionally set QT_IM_MODULE).
if(OPENKEYBOARD_BUILD_PLATFORM_PLUGIN)
    target_compile_definitions(openkeyboard INTERFACE OPENKEYBOARD_HAS_PLATFORM_PLUGIN=1)
endif()

# Namespaced alias so hosts link via OpenKeyboard::openkeyboard, consistent
# with the find_package(OpenKeyboard) export namespace.
add_library(OpenKeyboard::openkeyboard ALIAS openkeyboard)

generate_export_header(openkeyboard
    EXPORT_FILE_NAME openkeyboard_export.h
    BASE_NAME OPENKEYBOARD
)

set_target_properties(openkeyboard PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME openkeyboard
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# ------------------------------------------------------------------
# PlatformInputContext plugin (QPA) — discovered via QT_IM_MODULE,
# not linked directly by consumers. Depends on libopenkeyboard.so.
# ------------------------------------------------------------------
if(OPENKEYBOARD_BUILD_PLATFORM_PLUGIN)
    find_package(Qt${QT_VERSION_MAJOR}Gui COMPONENTS Private QUIET)
    if(NOT TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
        message(WARNING "GuiPrivate not found, platform plugin requires QT += gui-private (qpa/qplatforminputcontext.h). Skipping plugin.")
    else()
        add_library(openkeyboard_platforminputcontext SHARED
            src/platforminputcontext/openkeyboardinputcontext.cpp
            src/platforminputcontext/openkeyboardinputcontext.h
            src/platforminputcontext/openkeyboardplugin.cpp
            src/platforminputcontext/openkeyboardplugin.h
        )
        target_include_directories(openkeyboard_platforminputcontext PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge
            ${CMAKE_CURRENT_BINARY_DIR}
        )
        target_link_libraries(openkeyboard_platforminputcontext PRIVATE
            openkeyboard
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::GuiPrivate
        )
        set_target_properties(openkeyboard_platforminputcontext PROPERTIES
            AUTOMOC ON
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${OpenKeyboard_PLUGINS_DIR}/platforminputcontexts"
            RUNTIME_OUTPUT_DIRECTORY "${OpenKeyboard_PLUGINS_DIR}/platforminputcontexts"
            BUILD_RPATH_USE_ORIGIN ON
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
        # RPATH so the plugin finds libopenkeyboard.so both in build tree and after install
        # Build:  plugins/platforminputcontexts/ -> ../../  == build/ (or custom OpenKeyboard_PLUGINS_DIR)
        # Install: lib/qt/plugins/platforminputcontexts/ -> ../../../ == lib/
        if(UNIX)
            set_target_properties(openkeyboard_platforminputcontext PROPERTIES
                BUILD_RPATH "$ORIGIN/../.."
                INSTALL_RPATH "$ORIGIN/../../..:$ORIGIN/../.."
            )
        endif()

        # Namespaced alias consistent with find_package(OpenKeyboard) exports.
        add_library(OpenKeyboard::openkeyboard_platforminputcontext ALIAS openkeyboard_platforminputcontext)
    endif()
endif()

# ------------------------------------------------------------------
# Install & package export (Qt5 + Qt6 unified)
# ------------------------------------------------------------------
if(OPENKEYBOARD_INSTALL)
    # Core shared library — exported as OpenKeyboard::openkeyboard for find_package consumers
    install(TARGETS openkeyboard
        EXPORT OpenKeyboardTargets
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    # Platform plugin — installed to Qt plugin dir, discovered at runtime via QT_IM_MODULE
    if(TARGET openkeyboard_platforminputcontext)
        install(TARGETS openkeyboard_platforminputcontext
            EXPORT OpenKeyboardTargets
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt/plugins/platforminputcontexts"
            RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/qt/plugins/platforminputcontexts"
        )
    endif()

    # Public headers + generated export header
    install(FILES
        src/bridge/openkeyboardbridge.h
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/OpenKeyboard"
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/openkeyboard_export.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/OpenKeyboard"
    )

    # QML files (Qt5 filesystem import; Qt6 also embeds via qt_add_qml_module but keep for compat)
    install(DIRECTORY qml/ DESTINATION "${CMAKE_INSTALL_QMLDIR}")
    install(FILES OpenKeyboard/qmldir DESTINATION "${CMAKE_INSTALL_QMLDIR}")
    install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/OpenKeyboard")

    install(EXPORT OpenKeyboardTargets
        FILE OpenKeyboardTargets.cmake
        NAMESPACE OpenKeyboard::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/OpenKeyboard"
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/OpenKeyboardConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenKeyboardConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/OpenKeyboard"
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_QMLDIR
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/OpenKeyboardConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/OpenKeyboardConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenKeyboardConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/OpenKeyboard"
    )
endif()

if(OPENKEYBOARD_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
