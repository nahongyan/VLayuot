# views.pro - 自定义视图库
#
# 包含 FlowView 等高级视图组件
# 输出：build/lib/views.dll (Windows) 或 libviews.so (Linux)

TEMPLATE = lib
TARGET   = views
QT      += core gui widgets

# 引入全局配置
include(../../common.pri)

# 库输出目录
DESTDIR = $$BUILD_ROOT/lib

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/views
MOC_DIR = $$VAYOUT_OBJ_BASE/views/moc
RCC_DIR = $$VAYOUT_OBJ_BASE/views/rcc
UI_DIR = $$VAYOUT_OBJ_BASE/views/ui

# 公共头文件路径
INCLUDEPATH += $$PWD/.. $$PWD

# 构建时导出符号
DEFINES += VIEWS_SHARED VIEWS_BUILD

# 依赖 vlayout 库
LIBS += -L$$BUILD_ROOT/lib -lvlayout

# ============================================================
# FlowView 模块
# ============================================================

HEADERS += \
    $$PWD/flowview/flowview.h \
    $$PWD/flowview/flowlayoutengine.h

SOURCES += \
    $$PWD/flowview/flowview.cpp \
    $$PWD/flowview/flowlayoutengine.cpp
