# common.pri - VLayout 全局公共配置
#
# 使用方式：include(../common.pri) 或 include(../../common.pri)
# $$PWD 始终指向本文件所在目录（项目根目录）

# ============================================================
# 项目路径
# ============================================================

# 项目根目录（common.pri 所在目录）
PROJECT_ROOT = $$PWD

# 构建根目录
BUILD_ROOT = $$PROJECT_ROOT/build

# ============================================================
# 编译配置
# ============================================================

CONFIG += c++17

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
}

# ============================================================
# 库配置
# ============================================================

# 使用共享库
DEFINES += VLAYOUT_SHARED

# 库头文件路径 (libs/ 目录，使得 #include <vlayout/framework.h> 生效)
VAYOUT_INCLUDEPATH = $$PROJECT_ROOT/libs

# 库文件输出路径
VAYOUT_LIBPATH = $$BUILD_ROOT/lib

# 中间文件路径基目录
VAYOUT_OBJ_BASE = $$BUILD_ROOT/obj
