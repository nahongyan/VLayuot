# debugger.pro - VLayout 调试器示例
#
# 输出：build/bin/debugger_demo.exe

TEMPLATE = app
TARGET = debugger_demo
QT += core gui widgets

# 引入全局配置
include(../../common.pri)

# 启用调试器模块
DEFINES += VLAYOUT_DEBUGGER

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/debugger_demo
MOC_DIR = $$VAYOUT_OBJ_BASE/debugger_demo/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/debugger_demo/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/debugger_demo/ui

# 链接 vlayout 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout

# 源文件
SOURCES += main.cpp
