# SPDX-License-Identifier: MIT
# Copyright (c) 2026 MTL
cmake_minimum_required(VERSION 3.16)
project(OpenKeyboard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ------------------------------------------------------------------
# Single-artifact design: everything (bridge, input context, QML module)
# lives in ONE shared library — the Qt platform input context plugin.
# Qt loads it by itself when QT_IM_MODULE=openkeyboard; loading it mounts
# the embedded QML resources and runs the bridge static initializer.
# Hosts therefore add_subdirectory() and set one env var — nothing else.
# ------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(OPENKEYBOARD_DEFAULT_EXAMPLE ON)
else()
    set(OPENKEYBOARD_DEFAULT_EXAMPLE OFF)
endif()

option(OPENKEYBOARD_BUILD_EXAMPLE "Build example app" ${OPENKEYBOARD_DEFAULT_EXAMPLE})
option(OPENKEYBOARD_INSTALL "Generate install rules (installs into a Qt tree — pass -DCMAKE_INSTALL_PREFIX=<Qt dir>)" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Gui Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Gui Qml)

set(OPENKEYBOARD_QML_FILES
    OpenKeyboard/OpenInputPanel.qml
    OpenKeyboard/OpenKeyboard.qml
    OpenKeyboard/InputEngine.qml
    OpenKeyboard/CandidateBar.qml
    OpenKeyboard/styles/KeyboardStyle.qml
    OpenKeyboard/styles/compact/Style.qml
    OpenKeyboard/styles/default/Style.qml
    OpenKeyboard/components/KeyButton.qml
    OpenKeyboard/components/TextKey.qml
    OpenKeyboard/components/FunctionKey.qml
    OpenKeyboard/components/ActionKey.qml
    OpenKeyboard/components/EnterKey.qml
    OpenKeyboard/components/SpaceMicKey.qml
    OpenKeyboard/layouts/QwertyLayout.qml
    OpenKeyboard/layouts/SymbolsLayout.qml
    OpenKeyboard/layouts/NumberLayout.qml
)

set(OPENKEYBOARD_PLUGIN openkeyboard_platforminputcontext)

# Where hosts run their executables. The plugin is placed under
# <this>/platforminputcontexts so Qt's application-dir plugin search finds
# it without QT_PLUGIN_PATH. Hosts with a different runtime layout use
# openkeyboard_deploy_to(<target>) instead.
set(OPENKEYBOARD_RUNTIME_ROOT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
if(NOT OPENKEYBOARD_RUNTIME_ROOT)
    set(OPENKEYBOARD_RUNTIME_ROOT "${CMAKE_BINARY_DIR}")
endif()

set(OPENKEYBOARD_SOURCES
    src/bridge/openkeyboardbridge.cpp
    src/bridge/openkeyboardbridge.h
    src/platforminputcontext/openkeyboardinputcontext.cpp
    src/platforminputcontext/openkeyboardinputcontext.h
    src/platforminputcontext/openkeyboardplugin.cpp
    src/platforminputcontext/openkeyboardplugin.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    set(BUILD_SHARED_LIBS ON)
    set(QT_NO_PRIVATE_MODULE_WARNING ON)
    qt_add_qml_module(${OPENKEYBOARD_PLUGIN}
        URI OpenKeyboard
        VERSION 1.0
        QML_FILES ${OPENKEYBOARD_QML_FILES}
        RESOURCE_PREFIX "/qt/qml"
        PLUGIN_TARGET ${OPENKEYBOARD_PLUGIN}
        CLASS_NAME OpenKeyboardPlugin
        NO_GENERATE_PLUGIN_SOURCE   # the plugin class below is the input context plugin
    )
    target_sources(${OPENKEYBOARD_PLUGIN} PRIVATE ${OPENKEYBOARD_SOURCES})
else()
    add_library(${OPENKEYBOARD_PLUGIN} SHARED
        ${OPENKEYBOARD_SOURCES}
        OpenKeyboard/openkeyboard.qrc
    )
endif()

target_link_libraries(${OPENKEYBOARD_PLUGIN} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
)

# The input context plugin is built against qpa/qplatforminputcontext.h —
# private Qt headers. Needed for compilation and for moc's preprocessing.
find_package(Qt${QT_VERSION_MAJOR}Gui COMPONENTS Private QUIET)
if(TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
    target_link_libraries(${OPENKEYBOARD_PLUGIN} PRIVATE Qt${QT_VERSION_MAJOR}::GuiPrivate)
else()
    message(WARNING "Qt${QT_VERSION_MAJOR}::GuiPrivate not found — qpa/qplatforminputcontext.h unavailable, build will fail")
endif()

target_include_directories(${OPENKEYBOARD_PLUGIN} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile-time source root for the build-tree QML import fallback injected
# by the bridge's static initializer (see openkeyboardbridge.cpp).
target_compile_definitions(${OPENKEYBOARD_PLUGIN} PRIVATE
    OPENKEYBOARD_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(${OPENKEYBOARD_PLUGIN} PROPERTIES
    AUTOMOC ON
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${OPENKEYBOARD_RUNTIME_ROOT}/platforminputcontexts"
    RUNTIME_OUTPUT_DIRECTORY "${OPENKEYBOARD_RUNTIME_ROOT}/platforminputcontexts"
)

# ------------------------------------------------------------------
# One-line deployment for hosts whose executables do not sit in
# CMAKE_BINARY_DIR / CMAKE_RUNTIME_OUTPUT_DIRECTORY: copies the plugin
# next to <target>'s output so Qt's application-dir search finds it.
#   openkeyboard_deploy_to(your_app)
# ------------------------------------------------------------------
function(openkeyboard_deploy_to target)
    if(NOT TARGET openkeyboard_platforminputcontext)
        message(FATAL_ERROR "openkeyboard_deploy_to: target openkeyboard_platforminputcontext not defined — call after add_subdirectory(OpenKeyboard)")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:openkeyboard_platforminputcontext>
            $<TARGET_FILE_DIR:${target}>/platforminputcontexts/
        COMMENT "Deploying OpenKeyboard platform input context plugin"
    )
endfunction()

# ------------------------------------------------------------------
# Install = drop into a Qt tree so Qt discovers plugin + QML natively:
#   cmake -B build -DOPENKEYBOARD_INSTALL=ON
#   cmake --install build --prefix /path/to/Qt/5.15.2/gcc_64
# ------------------------------------------------------------------
if(OPENKEYBOARD_INSTALL)
    install(TARGETS ${OPENKEYBOARD_PLUGIN}
        LIBRARY DESTINATION plugins/platforminputcontexts
        RUNTIME DESTINATION plugins/platforminputcontexts
    )
    install(DIRECTORY OpenKeyboard/ DESTINATION qml/OpenKeyboard FILES_MATCHING PATTERN "*.qml")
    install(FILES OpenKeyboard/qmldir DESTINATION qml/OpenKeyboard)
    install(DIRECTORY OpenKeyboard/icons DESTINATION qml/OpenKeyboard)
    install(FILES LICENSE DESTINATION qml/OpenKeyboard)
endif()

if(OPENKEYBOARD_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
