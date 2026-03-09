# flowview_demo.pro - FlowView 高性能虚拟化列表演示
#
# 输出：build/bin/flowview_demo.exe

TEMPLATE = app
TARGET = flowview_demo
QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/flowview_demo
MOC_DIR = $$VAYOUT_OBJ_BASE/flowview_demo/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/flowview_demo/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/flowview_demo/ui

# 链接 vlayout 和 views 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
DEFINES += VIEWS_SHARED
LIBS += -L$$VAYOUT_LIBPATH -lvlayout -lviews

# 演示源文件
SOURCES += \
    main.cpp \
    chat_delegate.cpp \
    chat_model.cpp

HEADERS += \
    chat_delegate.h \
    chat_model.h
