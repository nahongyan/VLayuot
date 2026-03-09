# debugger.pro - VLayout 布局调试工具
#
# 功能：
# - 可视化调试布局
# - 生成 Delegate 代码
# - 生成 Model 代码
# - 数据绑定配置
#
# 输出：build/bin/vlayout_debugger.exe

TEMPLATE = app
TARGET   = vlayout_debugger
QT      += core gui widgets

# 引入全局配置
include(../../common.pri)

# 可执行文件输出目录
DESTDIR = $$BUILD_ROOT/bin

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/debugger
MOC_DIR = $$VAYOUT_OBJ_BASE/debugger/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/debugger/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/debugger/ui

# 链接 vlayout 库
INCLUDEPATH += $$VAYOUT_INCLUDEPATH
LIBS += -L$$VAYOUT_LIBPATH -lvlayout

# 调试器源文件
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
    $$PWD/layout_item_graphics.cpp \
    $$PWD/main.cpp
