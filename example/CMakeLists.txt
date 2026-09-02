# SPDX-License-Identifier: MIT
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(openkeyboard_example main.cpp qml.qrc)

# Inject machine-independent paths so main.cpp has no hardcoded absolute paths:
#   OPENKEYBOARD_PLUGINS_DIR  - build-tree plugin root (QT_PLUGIN_PATH)
#   OPENKEYBOARD_SOURCE_ROOT  - source root for QML import path (contains OpenKeyboard/)
target_compile_definitions(openkeyboard_example PRIVATE
    OPENKEYBOARD_PLUGINS_DIR="${OpenKeyboard_PLUGINS_DIR}"
    OPENKEYBOARD_SOURCE_ROOT="${OpenKeyboard_SOURCE_DIR}"
)

# Link only the core third-party shared library; the platform plugin
# (openkeyboard_platforminputcontext) is discovered at runtime via
# QT_IM_MODULE=openkeyboard / QT_PLUGIN_PATH — no direct link needed.
target_link_libraries(openkeyboard_example PRIVATE
    openkeyboard
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Gui
)

# Third-party usage (after install):
#   find_package(OpenKeyboard REQUIRED)
#   target_link_libraries(myapp PRIVATE OpenKeyboard::openkeyboard)
# Run with platform plugin:
#   QT_IM_MODULE=openkeyboard QT_PLUGIN_PATH=<build>/plugins ./openkeyboard_example

target_include_directories(openkeyboard_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
