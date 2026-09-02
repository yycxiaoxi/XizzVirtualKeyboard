# SPDX-License-Identifier: MIT
# Copyright (c) 2026 MTL
cmake_minimum_required(VERSION 3.16)
project(XizzVirtualKeyboard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ------------------------------------------------------------------
# Single-artifact design: everything (bridge, input context, QML module)
# lives in ONE shared library — the Qt platform input context plugin.
# Qt loads it by itself when QT_IM_MODULE=xizzvirtualkeyboard; loading it mounts
# the embedded QML resources and runs the bridge static initializer.
# Hosts therefore add_subdirectory() and set one env var — nothing else.
# ------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(XIZZVIRTUALKEYBOARD_DEFAULT_EXAMPLE ON)
else()
    set(XIZZVIRTUALKEYBOARD_DEFAULT_EXAMPLE OFF)
endif()

option(XIZZVIRTUALKEYBOARD_BUILD_EXAMPLE "Build example app" ${XIZZVIRTUALKEYBOARD_DEFAULT_EXAMPLE})
option(XIZZVIRTUALKEYBOARD_INSTALL "Generate install rules (installs into a Qt tree — pass -DCMAKE_INSTALL_PREFIX=<Qt dir>)" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Gui Qml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Gui Qml)

set(XIZZVIRTUALKEYBOARD_QML_FILES
    XizzVirtualKeyboard/OpenInputPanel.qml
    XizzVirtualKeyboard/XizzVirtualKeyboard.qml
    XizzVirtualKeyboard/InputEngine.qml
    XizzVirtualKeyboard/CandidateBar.qml
    XizzVirtualKeyboard/ScrollArea.qml
    XizzVirtualKeyboard/styles/KeyboardStyle.qml
    XizzVirtualKeyboard/styles/compact/Style.qml
    XizzVirtualKeyboard/styles/default/Style.qml
    XizzVirtualKeyboard/components/KeyButton.qml
    XizzVirtualKeyboard/components/TextKey.qml
    XizzVirtualKeyboard/components/FunctionKey.qml
    XizzVirtualKeyboard/components/ActionKey.qml
    XizzVirtualKeyboard/components/EnterKey.qml
    XizzVirtualKeyboard/components/SpaceMicKey.qml
    XizzVirtualKeyboard/layouts/QwertyLayout.qml
    XizzVirtualKeyboard/layouts/SymbolsLayout.qml
    XizzVirtualKeyboard/layouts/NumberLayout.qml
)

set(XIZZVIRTUALKEYBOARD_PLUGIN xizzvirtualkeyboard_platforminputcontext)

# Where hosts run their executables. The plugin is placed under
# <this>/platforminputcontexts so Qt's application-dir plugin search finds
# it without QT_PLUGIN_PATH. Hosts with a different runtime layout use
# xizzvirtualkeyboard_deploy_to(<target>) instead.
set(XIZZVIRTUALKEYBOARD_RUNTIME_ROOT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
if(NOT XIZZVIRTUALKEYBOARD_RUNTIME_ROOT)
    set(XIZZVIRTUALKEYBOARD_RUNTIME_ROOT "${CMAKE_BINARY_DIR}")
endif()

set(XIZZVIRTUALKEYBOARD_SOURCES
    src/bridge/virtualkeyboardbridge.cpp
    src/bridge/virtualkeyboardbridge.h
    src/platforminputcontext/virtualkeyboardinputcontext.cpp
    src/platforminputcontext/virtualkeyboardinputcontext.h
    src/platforminputcontext/virtualkeyboardplugin.cpp
    src/platforminputcontext/virtualkeyboardplugin.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    set(BUILD_SHARED_LIBS ON)
    set(QT_NO_PRIVATE_MODULE_WARNING ON)
    qt_add_qml_module(${XIZZVIRTUALKEYBOARD_PLUGIN}
        URI XizzVirtualKeyboard
        VERSION 1.0
        QML_FILES ${XIZZVIRTUALKEYBOARD_QML_FILES}
        RESOURCE_PREFIX "/qt/qml"
        PLUGIN_TARGET ${XIZZVIRTUALKEYBOARD_PLUGIN}
        CLASS_NAME XizzVirtualKeyboardPlugin
        NO_GENERATE_PLUGIN_SOURCE   # the plugin class below is the input context plugin
    )
    target_sources(${XIZZVIRTUALKEYBOARD_PLUGIN} PRIVATE ${XIZZVIRTUALKEYBOARD_SOURCES})
else()
    add_library(${XIZZVIRTUALKEYBOARD_PLUGIN} SHARED
        ${XIZZVIRTUALKEYBOARD_SOURCES}
        XizzVirtualKeyboard/virtualkeyboard.qrc
    )
endif()

target_link_libraries(${XIZZVIRTUALKEYBOARD_PLUGIN} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
)

# The input context plugin is built against qpa/qplatforminputcontext.h —
# private Qt headers. Needed for compilation and for moc's preprocessing.
find_package(Qt${QT_VERSION_MAJOR}Gui COMPONENTS Private QUIET)
if(TARGET Qt${QT_VERSION_MAJOR}::GuiPrivate)
    target_link_libraries(${XIZZVIRTUALKEYBOARD_PLUGIN} PRIVATE Qt${QT_VERSION_MAJOR}::GuiPrivate)
else()
    message(WARNING "Qt${QT_VERSION_MAJOR}::GuiPrivate not found — qpa/qplatforminputcontext.h unavailable, build will fail")
endif()

target_include_directories(${XIZZVIRTUALKEYBOARD_PLUGIN} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile-time source root for the build-tree QML import fallback injected
# by the bridge's static initializer (see virtualkeyboardbridge.cpp).
target_compile_definitions(${XIZZVIRTUALKEYBOARD_PLUGIN} PRIVATE
    XIZZVIRTUALKEYBOARD_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(${XIZZVIRTUALKEYBOARD_PLUGIN} PROPERTIES
    AUTOMOC ON
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${XIZZVIRTUALKEYBOARD_RUNTIME_ROOT}/platforminputcontexts"
    RUNTIME_OUTPUT_DIRECTORY "${XIZZVIRTUALKEYBOARD_RUNTIME_ROOT}/platforminputcontexts"
)

# ------------------------------------------------------------------
# One-line deployment for hosts whose executables do not sit in
# CMAKE_BINARY_DIR / CMAKE_RUNTIME_OUTPUT_DIRECTORY: copies the plugin
# next to <target>'s output so Qt's application-dir search finds it.
#   xizzvirtualkeyboard_deploy_to(your_app)
# ------------------------------------------------------------------
function(xizzvirtualkeyboard_deploy_to target)
    if(NOT TARGET xizzvirtualkeyboard_platforminputcontext)
        message(FATAL_ERROR "xizzvirtualkeyboard_deploy_to: target xizzvirtualkeyboard_platforminputcontext not defined — call after add_subdirectory(XizzVirtualKeyboard)")
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:xizzvirtualkeyboard_platforminputcontext>
            $<TARGET_FILE_DIR:${target}>/platforminputcontexts/
        COMMENT "Deploying XizzVirtualKeyboard platform input context plugin"
    )
endfunction()

# ------------------------------------------------------------------
# Install = drop into a Qt tree so Qt discovers plugin + QML natively:
#   cmake -B build -DXIZZVIRTUALKEYBOARD_INSTALL=ON
#   cmake --install build --prefix /path/to/Qt/5.15.2/gcc_64
# ------------------------------------------------------------------
if(XIZZVIRTUALKEYBOARD_INSTALL)
    install(TARGETS ${XIZZVIRTUALKEYBOARD_PLUGIN}
        LIBRARY DESTINATION plugins/platforminputcontexts
        RUNTIME DESTINATION plugins/platforminputcontexts
    )
    install(DIRECTORY XizzVirtualKeyboard/ DESTINATION qml/XizzVirtualKeyboard FILES_MATCHING PATTERN "*.qml")
    install(FILES XizzVirtualKeyboard/qmldir DESTINATION qml/XizzVirtualKeyboard)
    install(DIRECTORY XizzVirtualKeyboard/icons DESTINATION qml/XizzVirtualKeyboard)
    install(FILES LICENSE DESTINATION qml/XizzVirtualKeyboard)
endif()

if(XIZZVIRTUALKEYBOARD_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
