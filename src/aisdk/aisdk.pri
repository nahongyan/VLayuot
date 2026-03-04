# aisdk.pri - CLI 适配器模块
#
# 使用方法: 在你的 .pro 文件中添加:
#   include(path/to/aisdk.pri)
#
# 这个模块只提供 CLI 适配器接口，用于对接成熟 CLI (Ollama, Crush, OpenCode 等)

# SDK 路径
AISDK_PATH = $$PWD

# 头文件路径
INCLUDEPATH += $$AISDK_PATH

# 适配器模块 (对接成熟 CLI)
HEADERS += \
    $$AISDK_PATH/adapters/adapter.h \
    $$AISDK_PATH/adapters/ollama_adapter.h \
    $$AISDK_PATH/adapters/crush_adapter.h \
    $$AISDK_PATH/adapters/opencode_adapter.h

SOURCES += \
    $$AISDK_PATH/adapters/adapter.cpp \
    $$AISDK_PATH/adapters/ollama_adapter.cpp \
    $$AISDK_PATH/adapters/crush_adapter.cpp \
    $$AISDK_PATH/adapters/opencode_adapter.cpp

# Qt 模块依赖
QT += network
