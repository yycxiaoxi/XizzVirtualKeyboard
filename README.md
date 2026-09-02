# qml-openkeyborad (OpenKeyboard)

MTL 开源 QML 虚拟键盘第三方库 — MIT 协议。基于 QML 运行时（LGPL）之上自研，CMake 管理，参考 Qt Virtual Keyboard 架构，按截图样式实现，可被 `ChargingPileTcuProject` 等 QML 项目复用。

## 许可

- 本库：**MIT** — `LICENSE`，`SPDX-License-Identifier: MIT`。允许商用、闭源分发，需保留版权与许可声明。
- Qt QML 运行时：LGPL。本库为应用层 QML，不修改 Qt 源码，不触发 LGPL 源码分发义务。
- `openkeyboard_platforminputcontext` 插件动态链接 `Qt5GuiPrivate`（QPA 私有 API，`qpa/qplatforminputcontext.h`），仅 `#include` 头文件、未复制或修改 Qt 源码；使用者以源码编译本库（`add_subdirectory`）或保持动态链接即满足 LGPL 可重链接要求，无需开放宿主业务源码。

## 集成

### add_subdirectory（推荐，源码复用，形同其他第三方库）

```cmake
# 1. 加入子工程（example/install 默认关闭；插件默认开启）
add_subdirectory(3rdparty/qml-openkeyborad)

# 2. 链接核心库（插件不链接，运行时由 QT_IM_MODULE 自动发现）
target_link_libraries(your_app PRIVATE OpenKeyboard::openkeyboard)
```

接入后可读对外变量（`CACHE` 变量，宿主作用域可见）：
- `OpenKeyboard_SOURCE_DIR` — 源码根
- `OpenKeyboard_QML_IMPORT_PATH` — 传给 `engine.addImportPath()` 的父目录（`import OpenKeyboard 1.0` 按 `<此目录>/OpenKeyboard/qmldir` 解析）
- `OpenKeyboard_QML_QRC` — `qml/openkeyboard.qrc` 绝对路径（可嵌套进宿主 `.qrc`）
- `OpenKeyboard_PLUGINS_DIR` — 构建树插件根（运行时 `QT_PLUGIN_PATH`；可通过 `-DOpenKeyboard_PLUGINS_DIR=...` 覆盖）

```cpp
// main.cpp — 必须在 QGuiApplication 之前
qputenv("QT_IM_MODULE", QByteArray("openkeyboard"));
qputenv("OPENKEYBOARD_STYLE", QByteArray("compact")); // compact | default
QGuiApplication app(argc, argv);
// 插件路径（构建态；安装态 Qt 会从默认插件目录自动发现）
app.addLibraryPath(QStringLiteral(OPENKEYBOARD_PLUGINS_DIR)); // 或由 CMake 注入
QQmlApplicationEngine engine;
engine.addImportPath(QStringLiteral(OPENKEYBOARD_QML_IMPORT_PATH)); // 或由 CMake 注入
engine.rootContext()->setContextProperty("openKeyboardBridge", OpenKeyboardBridge::instance());
```

```qml
// main.qml — 全局自动弹出，无需手动 show()
import OpenKeyboard 1.0
ApplicationWindow {
    OpenInputPanel { parent: Overlay.overlay; z: 9999; actionLabel: "搜索" }
}
```

### find_package（已安装）

```cmake
find_package(OpenKeyboard REQUIRED)
target_link_libraries(your_app PRIVATE OpenKeyboard::openkeyboard)
# engine.addImportPath(OpenKeyboard_QML_IMPORT_PATH)
```

### qml.qrc 兼容

宿主若仍用 `qml.qrc` 打包，可直接将 `qml/openkeyboard.qrc` 加入 `target_sources`（或 `add_subdirectory` 后用 `OpenKeyboard_QML_QRC` 变量嵌套引入）。

## 用法（全局输入法接管）

宿主只需在 `main.cpp` 设置 `QT_IM_MODULE=openkeyboard`，任意 `TextField/TextArea` 聚焦即自动弹出，无需手动 `show()`。

```qml
import OpenKeyboard 1.0
OpenInputPanel { parent: Overlay.overlay; actionLabel: "搜索"; onActionTriggered: console.log(text) }
```

- `Qt.inputMethod.visible / keyboardRectangle / hide()` 全局联动，`inputMethodHints` 自动切页（Digits/Email/HiddenText）。
- 底行 `123 → 符号 → 数字 → ABC` 循环，空格与动作键。
- 候选栏 `InputEngine.candidates` 可覆盖，默认 `["大家都在做","美味上新"]`。

## 视觉与主题

`OPENKEYBOARD_STYLE` 环境变量切主题：`compact`（截图还原 `bg #EDEFF2 / key #FFF / func #C9CDD3 / accent #00C7A0`）与 `default`（`accent #3A7BFF`），单例分别为 `styles/compact/Style.qml` 与 `styles/default/Style.qml`，`KeyboardStyle.qml` 为兼容保留。

## 构建

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/home/user/Qt/5.15.2/gcc_64
cmake --build build
```

Qt6 自动走 `qt_add_qml_module`；Qt5 走 INTERFACE + qmldir 安装。

## 示例

`example/` 独立可执行：普通/密码/数字/邮箱四种输入演示。

## 目录

见 `CMakeLists.txt` 与 `OpenKeyboard/qmldir`。`src/platforminputcontext/` 为 `QPlatformInputContext` 插件（keys `openkeyboard`），`src/bridge/` 为 `OpenKeyboardBridge` 单例；QML 仅通过 `QInputMethodEvent` 与聚焦对象交互，不直改 `text`。
