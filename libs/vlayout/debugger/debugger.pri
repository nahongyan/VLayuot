# debugger.pri - VLayout 调试器模块
#
# 通过 DEFINES += VLAYOUT_DEBUGGER 启用

INCLUDEPATH += $$PWD/..

HEADERS += \
    $$PWD/sandbox_item.h \
    $$PWD/sandbox_preview.h \
    $$PWD/sandbox_widget.h \
    $$PWD/sandbox_scene.h \
    $$PWD/container_graphics_item.h \
    $$PWD/layout_item_graphics.h

SOURCES += \
    $$PWD/sandbox_preview.cpp \
    $$PWD/sandbox_widget.cpp \
    $$PWD/sandbox_scene.cpp \
    $$PWD/container_graphics_item.cpp \
    $$PWD/layout_item_graphics.cpp
