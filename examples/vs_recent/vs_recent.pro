# vs_recent.pro - VS 风格最近项目列表示例
#
# 输出：build/bin/vs_recent.exe

TEMPLATE = app
TARGET = vs_recent
QT += core gui widgets

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/vs_recent
MOC_DIR = $$VAYOUT_OBJ_BASE/vs_recent/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/vs_recent/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/vs_recent/ui

# 链接 vlayout 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout

# 源文件
SOURCES += \
    main.cpp \
    recentprojectswindow.cpp \
    vs_model.cpp \
    vs_delegates.cpp \
    vs_components.cpp \
    vs_utils.cpp

# 头文件
HEADERS += \
    recentprojectswindow.h \
    vs_model.h \
    vs_delegates.h \
    vs_components.h \
    vs_utils.h \
    vs_roles.h \
    vs_theme.h
