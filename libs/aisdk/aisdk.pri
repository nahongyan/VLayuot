# aisdk.pri - AI SDK 库使用配置
#
# 在你的 .pro 文件中添加:
#   include(path/to/aisdk.pri)
#
# 这将链接预编译的 aisdk 库。

# 头文件路径
INCLUDEPATH += $$PWD

# 导入符号（使用库时需要）
DEFINES += AISDK_SHARED

# 链接预编译库
LIBS += -L$$PWD/../../build/lib -laisdk

# Qt 模块依赖
QT += core network
CONFIG += c++17
