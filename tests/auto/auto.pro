# auto.pro - VLayout 自动化测试
#
# 输出：build/bin/tst_auto.exe

TEMPLATE = app
TARGET   = tst_auto
QT      += testlib core gui widgets

CONFIG += console
CONFIG -= app_bundle

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/tst_auto
MOC_DIR = $$VAYOUT_OBJ_BASE/tst_auto/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/tst_auto/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/tst_auto/ui

SOURCES += \
    tst_delegate.cpp

# 链接 vlayout 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout
