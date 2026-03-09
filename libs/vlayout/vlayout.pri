# vlayout.pri - VLayout 库使用配置
#
# 在你的 .pro 文件中添加:
#   include(path/to/vlayout.pri)
#
# 这将链接预编译的 vlayout 库。
# 如果需要直接编译源码，使用 vlayout_source.pri

# 头文件路径 - 使得 #include <vlayout/framework.h> 生效
INCLUDEPATH += $$PWD/..

# 链接预编译库
LIBS += -L$$PWD/../../build/lib -lvlayout

# 预处理器定义
DEFINES += VLAYOUT_SHARED

# Qt 模块依赖
QT += core gui widgets
CONFIG += c++17

# 可选模块启用（取消注释以启用）
# DEFINES += VLAYOUT_DEBUGGER   # 启用调试器模块
# DEFINES += VLAYOUT_FLOWVIEW   # 启用 FlowView 模块
