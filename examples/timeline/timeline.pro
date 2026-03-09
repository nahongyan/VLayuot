# timeline.pro - AI 编程助手时间轴组件演示
#
# 输出：build/bin/timeline_demo.exe

TEMPLATE = app
TARGET = timeline_demo
QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/timeline_demo
MOC_DIR = $$VAYOUT_OBJ_BASE/timeline_demo/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/timeline_demo/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/timeline_demo/ui

# 链接 vlayout 和 views 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout -lviews

# 引入 AI SDK (CLI 适配器)
include(../aisdk/aisdk.pri)

# Timeline 组件源文件
SOURCES += \
    main.cpp \
    timeline_utils.cpp \
    timeline_components.cpp \
    timeline_model.cpp \
    timeline_delegate.cpp \
    timeline_widget.cpp \
    markdown_renderer.cpp \
    code_highlighter.cpp

HEADERS += \
    timeline_roles.h \
    timeline_theme.h \
    timeline_utils.h \
    timeline_components.h \
    timeline_model.h \
    timeline_delegate.h \
    timeline_widget.h \
    markdown_renderer.h \
    code_highlighter.h
