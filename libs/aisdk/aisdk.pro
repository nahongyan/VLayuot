# aisdk.pro - AI CLI 适配器库
#
# 提供与成熟 CLI 工具（Ollama、Crush、OpenCode 等）通信的统一接口
# 输出：build/lib/aisdk.dll (Windows) 或 libaisdk.so (Linux)

TEMPLATE = lib
TARGET   = aisdk
QT      += core network

# 引入全局配置
include(../../common.pri)

# 库输出目录
DESTDIR = $$BUILD_ROOT/lib

# 中间文件目录
OBJECTS_DIR = $$VAYOUT_OBJ_BASE/aisdk
MOC_DIR = $$VAYOUT_OBJ_BASE/aisdk/moc

# 公共头文件路径
INCLUDEPATH += $$PWD

# 构建时导出符号
DEFINES += AISDK_SHARED AISDK_BUILD

# ============================================================
# 适配器模块
# ============================================================

HEADERS += \
    $$PWD/global.h \
    $$PWD/adapters/adapter.h \
    $$PWD/adapters/ollama_adapter.h \
    $$PWD/adapters/crush_adapter.h \
    $$PWD/adapters/opencode_adapter.h

SOURCES += \
    $$PWD/adapters/adapter.cpp \
    $$PWD/adapters/ollama_adapter.cpp \
    $$PWD/adapters/crush_adapter.cpp \
    $$PWD/adapters/opencode_adapter.cpp
