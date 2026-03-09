# vlayout_source.pri - 直接编译 VLayout 源码
#
# 如果不想使用预编译库，可以在 .pro 文件中添加:
#   include(path/to/vlayout_source.pri)
#
# 这将直接编译 VLayout 源码到你的项目中。

QT += core gui widgets
CONFIG += c++17

# 头文件路径 - 使得 #include <vlayout/framework.h> 生效
INCLUDEPATH += $$PWD/..

# 核心模块源码
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

# 可选模块
contains(DEFINES, VLAYOUT_DEBUGGER) {
    include($$PWD/debugger/debugger.pri)
}

contains(DEFINES, VLAYOUT_FLOWVIEW) {
    include($$PWD/flowview/flowview.pri)
}
