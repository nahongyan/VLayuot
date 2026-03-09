# VLayout.pro
# VLayout 声明式 Qt Delegate 框架 - 顶层项目文件
#
# 输出目录：
#   build/lib/   - 库文件
#   build/bin/   - 可执行文件
#   build/obj/   - 中间文件

TEMPLATE = subdirs

SUBDIRS += \
    libs/vlayout \
    libs/views \
    tools \
    tests \
    examples

# 依赖关系
libs/views.depends = libs/vlayout
tools.depends = libs/vlayout
tests.depends = libs/vlayout libs/views
examples.depends = libs/vlayout libs/views
