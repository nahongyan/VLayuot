# vlayout.pro - VLayout 核心库
#
# 声明式 Qt Model/View 委托框架
# 输出：build/lib/vlayout.dll (Windows) 或 libvlayout.so (Linux)

TEMPLATE = lib
TARGET   = vlayout
QT      += core gui widgets

# 引入全局配置
include(../../common.pri)

# 库输出目录
DESTDIR = $$BUILD_ROOT/lib

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/vlayout
MOC_DIR = $$VAYOUT_OBJ_BASE/vlayout/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/vlayout/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/vlayout/ui

# 公共头文件路径 - 使得 #include <vlayout/xxx.h> 生效
INCLUDEPATH += $$PWD/..

# 构建时导出符号
DEFINES += VLAYOUT_SHARED VLAYOUT_BUILD

# ============================================================
# 核心模块
# ============================================================

HEADERS += \
    $$PWD/global.h \
    $$PWD/framework.h \
    $$PWD/component.h \
    $$PWD/components.h \
    $$PWD/binding.h \
    $$PWD/layoutdescriptor.h \
    $$PWD/delegatecontroller.h \
    $$PWD/layoutitem.h \
    $$PWD/boxlayout.h \
    $$PWD/spaceritem.h \
    $$PWD/widgetitem.h

SOURCES += \
    $$PWD/component.cpp \
    $$PWD/components.cpp \
    $$PWD/delegatecontroller.cpp \
    $$PWD/layoutitem.cpp \
    $$PWD/boxlayout.cpp \
    $$PWD/spaceritem.cpp \
    $$PWD/widgetitem.cpp
