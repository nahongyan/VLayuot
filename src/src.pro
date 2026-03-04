# src.pro - VLayout 核心库
#
# 输出：build/lib/vlayout.dll (Windows) 或 libvlayout.so (Linux)

TEMPLATE = lib
TARGET   = vlayout
QT      += core gui widgets

# 引入全局配置
include(../common.pri)

# 库输出目录
DESTDIR = $$BUILD_ROOT/lib

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/vlayout
MOC_DIR = $$VAYOUT_OBJ_BASE/vlayout/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/vlayout/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/vlayout/ui

# 源文件
SOURCES += \
    vlayout/boxlayout.cpp \
    vlayout/component.cpp \
    vlayout/components.cpp \
    vlayout/delegatecontroller.cpp \
    vlayout/layoutitem.cpp \
    vlayout/spaceritem.cpp \
    vlayout/widgetitem.cpp

# 头文件
HEADERS += \
    vlayout/framework.h \
    vlayout/global.h \
    vlayout/binding.h \
    vlayout/component.h \
    vlayout/components.h \
    vlayout/delegatecontroller.h \
    vlayout/layoutdescriptor.h \
    vlayout/boxlayout.h \
    vlayout/layoutitem.h \
    vlayout/spaceritem.h \
    vlayout/widgetitem.h

# 公共头文件路径
INCLUDEPATH += $$PWD

DEFINES += VLAYOUT_SHARED VLAYOUT_BUILD VLAYOUT_DEBUGGER

# 调试器模块（Debug 构建时自动启用）
CONFIG(debug, debug|release) {
    DEFINES += VLAYOUT_DEBUGGER
}

contains(DEFINES, VLAYOUT_DEBUGGER) {
    SOURCES += \
        vlayout/debugger/sandbox_preview.cpp \
        vlayout/debugger/sandbox_widget.cpp

    HEADERS += \
        vlayout/debugger/sandbox_preview.h \
        vlayout/debugger/sandbox_widget.h
}
