# VLayout 调试器示例

TEMPLATE = app
TARGET = debugger_demo
QT += core gui widgets

CONFIG += c++17

# 启用调试器（必须在 include vlayout.pri 之前定义）
DEFINES += VLAYOUT_DEBUGGER

# 引入 VLayout 库
include(../../src/vlayout/vlayout.pri)

# 调试器源文件
SOURCES += \
    $$PWD/../../src/vlayout/debugger/sandbox_preview.cpp \
    $$PWD/../../src/vlayout/debugger/sandbox_widget.cpp

HEADERS += \
    $$PWD/../../src/vlayout/debugger/sandbox_preview.h \
    $$PWD/../../src/vlayout/debugger/sandbox_widget.h

# 主程序
SOURCES += main.cpp
