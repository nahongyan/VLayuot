# VLayout.pro
# VLayout 声明式 Qt Delegate 框架 - 顶层项目文件
#
# 构建方式：
#   qmake VLayout.pro
#   make
#
# 输出目录：
#   build/lib/   - 库文件
#   build/bin/   - 可执行文件
#   build/obj/   - 中间文件

TEMPLATE = subdirs

SUBDIRS += \
    libs/vlayout \
    tests \
    examples

# 依赖关系：tests 和 examples 依赖 vlayout
tests.depends = libs/vlayout
examples.depends = libs/vlayout
