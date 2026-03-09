# VLayout

声明式 Qt Delegate 框架

> 使用声明式 API 构建美观、数据驱动的列表视图

[![Qt Version](https://img.shields.io/badge/Qt-5.12+-41CD52?style=flat-square)](https://www.qt.io)
[![C++ Version](https://img.shields.io/badge/C++-11-00599C?style=flat-square)](https://isocpp.org)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square)](#特性)

**中文** | [English](README.md)

---

## 特性

- **声明式 API** - 在构造函数中声明式地描述 UI 布局和数据绑定
- **组件化架构** - 20+ 内置组件，易于扩展
- **流式数据绑定** - 链式 API 将 Model 数据绑定到组件属性
- **强大的布局引擎** - QBoxLayout 风格的水平/垂直布局，支持对齐
- **布局调试器** - 可视化调试工具，检查布局
- **高性能** - 布局结果缓存，快速路径优化
- **零依赖** - 仅依赖 Qt Core、Gui 和 Widgets 模块

## 快速开始

### 安装

**方式一：构建为库（推荐）**

```bash
git clone https://github.com/nahongyan/VLayout.git
cd vlayout
qmake VLayout.pro
make
```

**方式二：包含源码**

将 `libs/vlayout` 复制到你的项目，并在 `.pro` 文件中添加：

```qmake
include(libs/vlayout/vlayout_source.pri)
```

### 最小示例

```cpp
#include <vlayout/framework.h>
using namespace VLayout;

class MyDelegate : public DelegateController
{
public:
    MyDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        // 1. 定义布局
        addItem<LabelComponent>("title", -1);   // 弹性填充
        addItem<ButtonComponent>("btn", 60);    // 固定 60px
        addSpacing(8);
        setRowHeight(36);

        // 2. 数据绑定
        bindTo("title")
            .text(Qt::DisplayRole)
            .color(QColor(50, 50, 50));

        // 3. 事件处理
        onClick("btn", [](const QModelIndex& idx, IComponent*) {
            qDebug() << "点击:" << idx.data();
        });
    }
};
```

## 核心概念

### 布局系统

VLayout 提供与 QBoxLayout 兼容的布局引擎：

```cpp
// 基本行布局
setMargins(16, 8, 16, 8);  // 左, 上, 右, 下
setSpacing(12);             // 组件间距

addItem<IconComponent>("icon", 32, Qt::AlignVCenter);  // 固定 32px，垂直居中
addItem<LabelComponent>("title", -1);                   // 弹性填充
addItem<BadgeComponent>("count", 24);                   // 固定 24px
addSpacing(8);                                          // 间隔
```

### 对齐行为

| 对齐方式 | 主方向 | 交叉方向 |
| ------- | ------ | -------- |
| 无对齐 | 使用布局分配 | 填满容器 |
| AlignVCenter (HBox) | 使用布局分配 | 使用 sizeHint |
| AlignHCenter (VBox) | 使用布局分配 | 使用 sizeHint |
| AlignCenter | 使用 sizeHint | 使用 sizeHint |

```cpp
// 图标：32x32，垂直居中
auto icon = std::make_shared<WidgetItem>("icon");
icon->setSizeHint(QSize(32, 32));
icon->setMinimumSize(QSize(32, 32));
icon->setMaximumSize(QSize(32, 32));
icon->setAlignment(Qt::AlignVCenter);

// 文本：宽度拉伸，高度 24，垂直居中
auto text = std::make_shared<WidgetItem>("text");
text->setSizeHint(QSize(100, 24));
text->setMinimumSize(QSize(50, 24));
text->setStretch(1);
text->setAlignment(Qt::AlignVCenter);
```

### 数据绑定

```cpp
bindTo("icon")
    .property("iconType", IconTypeRole);

bindTo("title")
    .text(Qt::DisplayRole)
    .boldFont(12)
    .color(QColor(33, 33, 33));

bindTo("status")
    .text(StatusRole, [](const QVariant& v) {
        return statusText(v.toInt());
    })
    .color(StatusRole, [](const QVariant& v) {
        return v.toInt() > 0 ? QColor(76, 175, 80) : QColor(158, 158, 158);
    })
    .visibleWhenNotEmpty(StatusRole);

bindTo("checkbox")
    .checkedWhenTrue(CheckedRole)
    .onClick([](const QModelIndex& idx, IComponent*) {
        toggleData(idx, CheckedRole);
    });
```

## 组件库

### 内置组件

| 组件 | 描述 |
| --- | --- |
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

## 扩展

### 创建自定义组件

继承 `AbstractComponent`：

```cpp
#include <vlayout/component.h>

class ProgressCircle : public AbstractComponent
{
public:
    explicit ProgressCircle(const QString& id) : AbstractComponent(id) {}

    QString type() const override { return "ProgressCircle"; }

    void paint(ComponentContext& ctx) override
    {
        QPainter* p = ctx.painter;
        QRect r = geometry();
        int value = m_value;

        // 背景圆
        p->setPen(QPen(QColor(200, 200, 200), 3));
        p->drawEllipse(r.center(), r.width()/2 - 3, r.height()/2 - 3);

        // 进度弧
        if (value > 0) {
            p->setPen(QPen(QColor(76, 175, 80), 3));
            int angle = value * 360 / 100 * 16;
            p->drawArc(r.adjusted(3, 3, -3, -3), 90 * 16, -angle);
        }

        // 百分比文本
        p->drawText(r, Qt::AlignCenter, QString("%1%").arg(value));
    }

    void setValue(int v) { m_value = qBound(0, v, 100); update(); }
    int value() const { return m_value; }

private:
    int m_value = 0;
};
```

### 注册自定义组件

```cpp
class MyDelegate : public DelegateController
{
public:
    MyDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        // 注册自定义组件
        registerComponent<ProgressCircle>("progress");
        addItem<ProgressCircle>("progress", 40, Qt::AlignVCenter);

        // 绑定数据
        bindTo("progress")
            .property("value", ProgressRole);
    }
};
```

### 布局项类型

VLayout 使用三种布局项类型：

```cpp
// WidgetItem - 用于可视组件
auto widget = std::make_shared<WidgetItem>("id");
widget->setSizeHint(QSize(100, 30));
widget->setMinimumSize(QSize(50, 20));
widget->setMaximumSize(QSize(200, 50));
widget->setStretch(1);
widget->setAlignment(Qt::AlignVCenter);

// SpacerItem - 用于固定间距
auto spacer = std::make_shared<SpacerItem>(20, 10);  // 宽度, 高度

// BoxLayout - 用于嵌套布局
auto innerLayout = std::make_shared<VBoxLayout>();
innerLayout->addItem(widget1);
innerLayout->addItem(widget2);
outerLayout->addItem(innerLayout);
```

## 布局调试器

VLayout 包含可视化调试工具：

```cpp
#include <vlayout/debugger/sandbox_widget.h>

// 创建调试窗口
auto debugger = new VLayout::LayoutSandboxWidget();
debugger->setWindowTitle("布局调试器");
debugger->show();

// 或加载现有布局
debugger->loadLayout(myBoxLayout);
```

**功能：**

- 网格背景可视化预览
- 实时参数调整
- 添加/删除布局项
- 导出/导入布局为 JSON

## 示例

### VS 风格项目列表项

```cpp
class ProjectItemDelegate : public DelegateController
{
public:
    ProjectItemDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        setMargins(24, 8, 12, 8);
        setSpacing(8);

        addItem<SolutionIconComponent>("icon", 36, Qt::AlignVCenter);
        addItem<ProjectInfoComponent>("info", -1);
        addItem<DateComponent>("date", 120, Qt::AlignVCenter);
        addItem<PinComponent>("pin", 24, Qt::AlignVCenter);

        setRowHeight(52);

        bindTo("icon").property("iconType", IconTypeRole);
        bindTo("info").property("name", NameRole).property("path", PathRole);
        bindTo("date").property("date", DateRole);
        bindTo("pin").checkedWhenTrue(PinnedRole);

        onClick("pin", [](const QModelIndex& idx, IComponent*) {
            toggleData(idx, PinnedRole);
        });
    }
};
```

### 聊天消息

```cpp
class ChatMessageDelegate : public DelegateController
{
public:
    ChatMessageDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        setMargins(16, 12, 16, 12);
        setSpacing(12);

        addItem<AvatarComponent>("avatar", 40, Qt::AlignTop);
        addItem<MessageContentComponent>("content", -1);

        bindTo("avatar")
            .property("imageUrl", AvatarUrlRole)
            .property("initials", NameRole);

        bindTo("content")
            .property("text", MessageRole)
            .property("time", TimeRole)
            .property("isOwn", IsOwnMessageRole);
    }
};
```

## API 参考

### DelegateController

```cpp
// 组件
template<typename T>
T* addItem(const QString& id, int width = -1, Qt::Alignment align = {});

// 布局
void setMargins(int left, int top, int right, int bottom);
void setSpacing(int spacing);
void setRowHeight(int height);
void addSpacing(int width);
void addStretch(int stretch = 1);

// 数据绑定
BindingBuilder bindTo(const QString& componentId);

// 事件
void onClick(const QString& id, ClickHandler handler);

// 工具方法
static void toggleData(const QModelIndex& idx, int role);
static void setModelData(const QModelIndex& idx, const QVariant& value, int role);
```

### BindingBuilder

```cpp
bindTo("component")
    .display()                          // Qt::DisplayRole
    .text(int role)                     // 绑定文本
    .text(int role, Converter conv)     // 带转换器
    .font(const QFont& font)            // 固定字体
    .boldFont(int pointSize)            // 粗体字体
    .color(const QColor& color)         // 固定颜色
    .color(int role, ColorConverter)    // 条件颜色
    .property(const QString& name, int role)
    .visibleWhen(int role, Condition)
    .visibleWhenNotEmpty(int role)
    .checkedWhenTrue(int role)
    .onClick(ClickHandler);
```

## 项目结构

```text
VLayout/
├── libs/
│   ├── vlayout/           # 核心库
│   │   ├── framework.h    # 主包含文件
│   │   ├── component.h    # 组件接口
│   │   ├── components.h   # 内置组件
│   │   ├── binding.h      # 数据绑定
│   │   ├── delegatecontroller.h
│   │   ├── boxlayout.h    # 布局引擎
│   │   ├── layoutitem.h   # 布局项基类
│   │   ├── widgetitem.h   # Widget 项
│   │   ├── spaceritem.h   # Spacer 项
│   │   ├── debugger/      # 调试工具
│   │   └── flowview/      # 高性能虚拟化列表
│   └── aisdk/             # AI CLI 适配器（可选）
│
├── tests/                 # 单元测试
├── examples/              # 示例
└── docs/                  # 文档
```

## 贡献

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 许可证

MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

---

用 ❤️ 为 Qt 开发者制作
