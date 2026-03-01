# VLayout 项目架构重构设计

## 概述

将 VLayout 项目从扁平结构重构为标准分层子项目工程结构。

## 设计决策

| 项目 | 选择 |
|------|------|
| 构建系统 | qmake (.pro) |
| 目录结构 | 标准分层 (src/tests/examples) |
| 测试框架 | Qt Test |
| 顶层管理 | subdirs 模板 |
| 示例组织 | 独立子项目工程 |
| 脚本文件 | 删除 |

## 最终目录结构

```
stylegenerage/
├── VLayout.pro                  # 顶层 subdirs 项目
├── README.md                    # 更新文档
├── docs/                        # 保留现有文档
│
├── src/
│   ├── src.pro                  # lib 模板
│   └── vlayout/                 # 核心库源码
│       ├── framework.h
│       ├── global.h
│       ├── binding.h
│       ├── component.h/.cpp
│       ├── components.h/.cpp
│       ├── delegatecontroller.h/.cpp
│       ├── layoutdescriptor.h
│       ├── boxlayout.h/.cpp
│       ├── layoutitem.h/.cpp
│       ├── widgetitem.h/.cpp
│       ├── spaceritem.h/.cpp
│       └── vlayout.pri
│
├── tests/
│   ├── tests.pro                # subdirs
│   └── auto/
│       ├── auto.pro
│       └── tst_delegate.cpp
│
└── examples/
    ├── examples.pro             # subdirs
    ├── vs_recent/
    │   ├── vs_recent.pro
    │   └── (源文件)
    └── timeline/
        ├── timeline.pro
        └── (源文件)
```

## 项目文件配置

### VLayout.pro（顶层）

```qmake
TEMPLATE = subdirs
CONFIG   = ordered

SUBDIRS += \
    src \
    tests \
    examples

tests.depends = src
examples.depends = src
```

### src/src.pro

```qmake
TEMPLATE = lib
TARGET   = vlayout
QT      += core gui

SOURCES += \
    vlayout/boxlayout.cpp \
    vlayout/component.cpp \
    vlayout/components.cpp \
    vlayout/delegatecontroller.cpp \
    vlayout/layoutitem.cpp \
    vlayout/spaceritem.cpp \
    vlayout/widgetitem.cpp

HEADERS += \
    vlayout/framework.h \
    vlayout/global.h \
    vlayout/binding.h \
    vlayout/component.h \
    vlayout/components.h \
    vlayout/delegatecontroller.h \
    vlayout/layoutdescriptor.h \
    vlayout/boxlayout.h \
    vlayout/layoutitem.h \
    vlayout/spaceritem.h \
    vlayout/widgetitem.h

INCLUDEPATH += $$PWD
```

### tests/tests.pro

```qmake
TEMPLATE = subdirs
SUBDIRS += auto
```

### tests/auto/auto.pro

```qmake
TEMPLATE = app
TARGET   = tst_auto
QT      += testlib core gui

SOURCES += tst_delegate.cpp

INCLUDEPATH += ../../src
LIBS += -L../../src -lvlayout
```

### examples/examples.pro

```qmake
TEMPLATE = subdirs

SUBDIRS += \
    vs_recent \
    timeline
```

## 迁移操作清单

1. 创建 src/ 目录结构
2. 移动 vlayout/ 源码到 src/vlayout/
3. 创建 src/src.pro
4. 创建 tests/ 目录结构
5. 创建测试模板文件
6. 创建 examples/ 目录结构
7. 移动 demo/vs_recent/ 到 examples/vs_recent/
8. 移动 demo/timeline/ 到 examples/timeline/
9. 更新示例项目的 .pro 文件
10. 创建顶层 VLayout.pro
11. 删除 demo/ 目录及构建产物
12. 删除根目录脚本文件
13. 更新 README.md

## 日期

2026-03-01
