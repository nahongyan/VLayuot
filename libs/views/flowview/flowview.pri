# flowview.pri - VLayout FlowView 模块
#
# 高性能虚拟化列表组件
# 通过 DEFINES += VLAYOUT_FLOWVIEW 启用

INCLUDEPATH += $$PWD/..

HEADERS += \
    $$PWD/flowview.h \
    $$PWD/flowlayoutengine.h

SOURCES += \
    $$PWD/flowview.cpp \
    $$PWD/flowlayoutengine.cpp
