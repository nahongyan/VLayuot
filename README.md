# VLayout Framework

<p align="center">
  <strong>声明式 Qt Delegate 框架</strong><br>
  <em>Declarative Delegate Framework for Qt Model/View</em>
</p>

<p align="center">
  <a href="#特性">特性</a> •
  <a href="#快速开始">快速开始</a> •
  <a href="#核心概念">核心概念</a> •
  <a href="#组件库">组件库</a> •
  <a href="#示例">示例</a> •
  <a href="#许可证">许可证</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Qt-5.12+-green.svg" alt="Qt Version">
  <img src="https://img.shields.io/badge/C++-11-blue.svg" alt="C++ Version">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License">
</p>

---

## 特性

- **声明式 API** - 在构造函数中声明式地描述 UI 布局和数据绑定
- **组件化架构** - 可复用的 UI 组件，支持自定义扩展
- **流式数据绑定** - 链式 API 将 Model 数据绑定到组件属性
- **事件处理** - 声明式点击事件处理，无需手动实现 `editorEvent`
- **布局引擎** - 支持 QBoxLayout 风格的水平/垂直布局，带缓存优化
- **丰富的组件库** - 内置 20+ 常用组件（Label, Button, CheckBox, ProgressBar 等）
- **零依赖** - 仅依赖 Qt Core 和 Qt Gui 模块
- **高性能** - 布局结果缓存，快速路径优化

---

## 快速开始

### 安装

#### 方式一：作为库使用（推荐）

1. 构建整个项目：
   ```bash
   qmake VLayout.pro
   make
   ```

2. 输出目录结构：
   ```
   build/
   ├── lib/           # 库文件 (vlayout.dll/.a)
   ├── bin/           # 可执行文件 (示例、测试)
   └── obj/           # 中间文件
   ```

3. 在你的项目中链接：
   ```qmake
   INCLUDEPATH += /path/to/VLayout/src
   LIBS += -L/path/to/VLayout/build/lib -lvlayout
   ```

#### 方式二：直接包含源码

将 `src/vlayout` 目录复制到你的项目中，并在 `.pro` 文件中添加：

```qmake
include(src/vlayout/vlayout.pri)
```

### 最小示例

```cpp
#include <vlayout/framework.h>
using namespace VLayout;

// 自定义 Delegate
class MyDelegate : public DelegateController
{
public:
    MyDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        // 添加组件
        addItem<LabelComponent>("title", -1);  // 弹性填充
        addItem<ButtonComponent>("btn", 60);    // 固定 60px
        addSpacing(8);
        setRowHeight(36);

        // 数据绑定
        bindTo("title")
            .text(Qt::DisplayRole)
            .color(QColor(232, 232, 240));

        // 事件处理
        onClick("btn", [](const QModelIndex& idx, IComponent*) {
            qDebug() << "Clicked:" << idx.data().toString();
        });
    }
};
```

---

## 核心概念

### DelegateController

`DelegateController` 是框架的核心类，继承自 `QStyledItemDelegate`。它提供声明式 API 来配置：

- **组件管理** - `addComponent()`, `addItem<T>()`
- **布局描述** - `setLayout()`, `setRow()`, `addItem()`, `addSpacing()`
- **数据绑定** - `bindTo()`
- **事件处理** - `onClick()`

### 组件 (Component)

组件是可绘制的 UI 元素，继承自 `IComponent` 接口。框架提供 `AbstractComponent` 基类，简化自定义组件的开发。

```cpp
class MyComponent : public AbstractComponent
{
public:
    MyComponent(const QString& id) : AbstractComponent(id) {}

    QString type() const override { return "MyComponent"; }

    void paint(ComponentContext& ctx) override {
        QPainter* p = ctx.painter;
        QRect r = geometry();
        p->drawText(r, Qt::AlignCenter, text());
    }

    void setText(const QString& t) { setProperty("text", t); }
    QString text() const { return property("text").toString(); }
};
```

### 数据绑定

使用 `bindTo()` 创建从 Model 到组件的声明式绑定：

```cpp
bindTo("icon")
    .property("iconType", IconTypeRole)
    .property("name", NameRole);

bindTo("title")
    .text(Qt::DisplayRole)
    .boldFont(12)
    .color(255, 255, 255);

bindTo("badge")
    .text(StatusRole, [](const QVariant& v) {
        return statusText(v.toInt());
    })
    .color(StatusRole, [](const QVariant& v) {
        return v.toInt() > 0 ? QColor(76, 175, 80) : QColor(158, 158, 158);
    })
    .visibleWhenNotEmpty(StatusRole);

bindTo("pin")
    .checkedWhenTrue(PinnedRole)
    .onClick([](const QModelIndex& idx, IComponent*) {
        toggleData(idx, PinnedRole);
    });
```

### 布局系统

#### 极简行布局 API

类似 `QBoxLayout` 的流式 API：

```cpp
setMargins(16, 8, 16, 8);  // 左, 上, 右, 下
setSpacing(12);             // 组件间距

addItem<SpacerComponent>("indent");         // sizeHint 决定宽度
addItem<ExpandArrowComponent>("arrow", 16); // 固定 16px
addItem<LabelComponent>("name", -1);        // 弹性填充
addItem<BadgeComponent>("count", 24);       // 固定 24px
addSpacing(8);                              // 右侧留白

setRowHeight(36);
```

#### 高级嵌套布局

使用声明式描述符创建复杂布局：

```cpp
setLayout(HBox(16, 8, 16, 8, 12, {
    Item("avatar", {40, 40}),
    Stretch("info"),
    VBox({Item("title"), Item("subtitle")}),
    Item("action", {24, 24}),
}));
```

---

## 组件库

| 组件 | 描述 |
|------|------|
| `LabelComponent` | 文本标签，支持对齐、换行、省略 |
| `ButtonComponent` | 按钮，支持文本、图标、可选中 |
| `CheckBoxComponent` | 复选框 |
| `SwitchComponent` | 开关 |
| `ProgressBarComponent` | 进度条 |
| `CircularProgressComponent` | 圆形进度指示器 |
| `SliderComponent` | 滑动条 |
| `SpinBoxComponent` | 数值调节框 |
| `ComboBoxComponent` | 下拉框 |
| `IconComponent` | 图标（图标字体字符） |
| `ImageComponent` | 图片，支持圆角 |
| `AvatarComponent` | 头像，显示图片或首字母 |
| `BadgeComponent` | 徽章/标记 |
| `RatingComponent` | 星级评分 |
| `SeparatorComponent` | 分隔线 |
| `CardComponent` | 卡片容器 |
| `SpacerComponent` | 占位符 |
| `ExpandArrowComponent` | 展开/折叠箭头 |
| `DecorationIconComponent` | QIcon 装饰图标 |

---

## 示例

### Visual Studio 风格的最近项目列表

完整示例位于 `demo/vs_recent/` 目录。

```cpp
class ProjectItemDelegate : public VLayout::DelegateController
{
public:
    ProjectItemDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        // 布局：[图标(36px)] [项目信息(stretch)] [日期(120px)] [图钉(24px)]
        setMargins(24, 8, 12, 8);
        setSpacing(8);

        addItem<SolutionIconComponent>("icon", 36, Qt::AlignVCenter);
        addItem<ProjectInfoComponent>("info", -1);
        addItem<DateComponent>("date", 120, Qt::AlignVCenter);
        addItem<PinComponent>("pin", 24, Qt::AlignVCenter);

        setRowHeight(52);

        // 数据绑定
        bindTo("icon")
            .property("iconType", VS::IconTypeRole)
            .property("name", VS::NameRole);

        bindTo("info")
            .property("name", VS::NameRole)
            .property("path", VS::PathRole);

        bindTo("date")
            .property("date", VS::DateRole);

        bindTo("pin")
            .checkedWhenTrue(VS::PinnedRole);

        // 点击切换固定状态
        onClick("pin", [](const QModelIndex& index, IComponent*) {
            toggleData(index, VS::PinnedRole);
        });
    }
};
```

---

## API 参考

### DelegateController

```cpp
// 组件管理
addComponent(comp)           // 注册组件
component(id)                // 获取组件
addItem<T>(id, width, align) // 注册并加入布局

// 布局
setLayout(descriptor)        // 高级嵌套布局
setRow({...})               // 极简行布局
addSpacing(width)           // 添加间距
addStretch(stretch)         // 添加弹簧
setMargins(left, top, right, bottom)
setSpacing(spacing)
setRowHeight(height)

// 数据绑定
bindTo(componentId)         // 返回 BindingBuilder
    .text(role)
    .font(role, converter)
    .color(role, converter)
    .property(name, role)
    .visibleWhen(role, condition)
    .checkedWhenTrue(role)
    .onClick(callback)

// 事件
onClick(componentId, handler)
onAnyClick(handler)

// 便捷方法
setFixedSizeHint(size)
toggleData(index, role)
setModelData(index, value, role)
```

### BindingBuilder

```cpp
bindTo("title")
    .display()                          // Qt::DisplayRole
    .text(RoleName)                     // 绑定文本
    .text(RoleStatus, converter)        // 带转换器
    .font(QFont("Segoe UI", 10))        // 固定字体
    .boldFont(12)                       // 粗体字体
    .color(QColor(255, 255, 255))       // 固定颜色
    .color(RoleStatus, colorConverter)  // 条件颜色
    .property("sizeHint", RoleSize)     // 任意属性
    .visibleWhenNotEmpty(RoleBadge)     // 非空时可见
    .checkedWhenTrue(RoleChecked)       // true 时选中
    .onClick(handler);                  // 点击事件
```

---

## 项目结构

```
VLayout/
├── VLayout.pro              # 顶层项目文件 (subdirs)
├── README.md
├── docs/                    # 文档
│   └── plans/               # 设计文档
│
├── src/                     # 库源码
│   ├── src.pro              # 库项目文件
│   └── vlayout/             # 核心框架
│       ├── framework.h           # 框架入口
│       ├── global.h              # 全局定义
│       ├── component.h/.cpp      # 组件接口和基类
│       ├── components.h/.cpp     # 内置组件库
│       ├── binding.h             # 数据绑定系统
│       ├── delegatecontroller.h/.cpp  # 核心 Delegate
│       ├── layoutdescriptor.h    # 布局描述符
│       ├── boxlayout.h/.cpp      # Box 布局引擎
│       ├── layoutitem.h/.cpp     # 布局项基类
│       ├── widgetitem.h/.cpp     # Widget 布局项
│       ├── spaceritem.h/.cpp     # Spacer 弹簧项
│       └── vlayout.pri           # qmake 包含文件
│
├── tests/                   # 单元测试
│   ├── tests.pro
│   └── auto/
│       └── tst_delegate.cpp
│
└── examples/                # 示例项目
    ├── examples.pro
    ├── vs_recent/           # VS 风格最近项目列表
    └── timeline/            # AI 编程助手时间轴
```

---

## 许可证

MIT License

Copyright (c) 2024

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

<p align="center">
  Made with ❤️ for Qt Developers
</p>
