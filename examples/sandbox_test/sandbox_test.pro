# sandbox_test.pro - VLayout 沙盒测试示例
#
# 输出：build/bin/sandbox_test.exe

TEMPLATE = app
TARGET   = sandbox_test
QT      += core gui widgets

CONFIG += console
CONFIG -= app_bundle

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/sandbox_test
MOC_DIR = $$VAYOUT_OBJ_BASE/sandbox_test/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/sandbox_test/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/sandbox_test/ui

SOURCES += \
    main.cpp

# 链接 vlayout 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout
