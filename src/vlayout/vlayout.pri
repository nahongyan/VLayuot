
QT += core gui widgets

CONFIG += c++17

# $$PWD = vlayout/ 目录本身；上级目录才能让 #include "vlayout/xxx.h" 正确解析
INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/global.h \
    $$PWD/layoutitem.h \
    $$PWD/boxlayout.h \
    $$PWD/spaceritem.h \
    $$PWD/widgetitem.h \
    $$PWD/component.h \
    $$PWD/binding.h \
    $$PWD/layoutdescriptor.h \
    $$PWD/delegatecontroller.h \
    $$PWD/components.h \
    $$PWD/framework.h

SOURCES += \
    $$PWD/layoutitem.cpp \
    $$PWD/boxlayout.cpp \
    $$PWD/spaceritem.cpp \
    $$PWD/widgetitem.cpp \
    $$PWD/component.cpp \
    $$PWD/delegatecontroller.cpp \
    $$PWD/components.cpp

# 调试器模块（通过 DEFINES += VLAYOUT_DEBUGGER 启用）
contains(DEFINES, VLAYOUT_DEBUGGER) {
    include($$PWD/debugger/debugger.pri)
}
