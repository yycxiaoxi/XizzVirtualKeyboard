# SPDX-License-Identifier: MIT
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Qml Quick Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Quick Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(openkeyboard_example main.cpp qml.qrc)

# Run from the build root so that Qt finds the plugin in
# <appdir>/platforminputcontexts with zero configuration. Hosts whose
# executables live elsewhere call openkeyboard_deploy_to(<target>) instead.
set_target_properties(openkeyboard_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Nothing from OpenKeyboard is linked — the plugin is loaded at runtime via
# QT_IM_MODULE=openkeyboard (set in main.cpp).
target_link_libraries(openkeyboard_example PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Gui
)
