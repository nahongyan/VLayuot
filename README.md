# VLayout

Declarative Delegate Framework for Qt

> Build beautiful, data-driven list views with declarative API

[![Qt Version](https://img.shields.io/badge/Qt-5.12+-41CD52?style=flat-square)](https://www.qt.io)
[![C++ Version](https://img.shields.io/badge/C++-11-00599C?style=flat-square)](https://isocpp.org)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square)](#features)

[中文文档](README_CN.md) | **English**

---

## Features

- **Declarative API** - Describe UI layout and data binding in constructor
- **Component-based** - 20+ built-in components, easy to extend
- **Fluent Data Binding** - Chain API to bind Model data to component properties
- **Powerful Layout Engine** - QBoxLayout-style H/V layouts with alignment support
- **Layout Debugger** - Visual debugging tool for layout inspection
- **High Performance** - Layout result caching, fast path optimization
- **Zero Dependency** - Only requires Qt Core, Gui, and Widgets

## Quick Start

### Installation

**Option 1: Build as Library (Recommended)**

```bash
git clone https://github.com/yourusername/vlayout.git
cd vlayout
qmake VLayout.pro
make
```

**Option 2: Include Source**

Copy `src/vlayout` to your project and add to `.pro`:

```qmake
include(src/vlayout/vlayout.pri)
```

### Minimal Example

```cpp
#include <vlayout/framework.h>
using namespace VLayout;

class MyDelegate : public DelegateController
{
public:
    MyDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        // 1. Define layout
        addItem<LabelComponent>("title", -1);   // Stretch fill
        addItem<ButtonComponent>("btn", 60);    // Fixed 60px
        addSpacing(8);
        setRowHeight(36);

        // 2. Data binding
        bindTo("title")
            .text(Qt::DisplayRole)
            .color(QColor(50, 50, 50));

        // 3. Event handling
        onClick("btn", [](const QModelIndex& idx, IComponent*) {
            qDebug() << "Clicked:" << idx.data();
        });
    }
};
```

## Core Concepts

### Layout System

VLayout provides a QBoxLayout-compatible layout engine:

```cpp
// Basic row layout
setMargins(16, 8, 16, 8);  // left, top, right, bottom
setSpacing(12);             // item spacing

addItem<IconComponent>("icon", 32, Qt::AlignVCenter);  // Fixed 32px, vertically centered
addItem<LabelComponent>("title", -1);                   // Stretch fill
addItem<BadgeComponent>("count", 24);                   // Fixed 24px
addSpacing(8);                                          // Spacer
```

### Alignment Behavior

| Alignment | Main Direction | Cross Direction |
| --------- | -------------- | --------------- |
| No alignment | Uses layout allocation | Fills container |
| AlignVCenter (HBox) | Uses layout allocation | Uses sizeHint |
| AlignHCenter (VBox) | Uses layout allocation | Uses sizeHint |
| AlignCenter | Uses sizeHint | Uses sizeHint |

```cpp
// Icon: 32x32, vertically centered
auto icon = std::make_shared<WidgetItem>("icon");
icon->setSizeHint(QSize(32, 32));
icon->setMinimumSize(QSize(32, 32));
icon->setMaximumSize(QSize(32, 32));
icon->setAlignment(Qt::AlignVCenter);

// Text: width stretches, height 24, vertically centered
auto text = std::make_shared<WidgetItem>("text");
text->setSizeHint(QSize(100, 24));
text->setMinimumSize(QSize(50, 24));
text->setStretch(1);
text->setAlignment(Qt::AlignVCenter);
```

### Data Binding

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

## Components

### Built-in Components

| Component | Description |
| --------- | ----------- |
| `LabelComponent` | Text label with alignment, word wrap, elide |
| `ButtonComponent` | Button with text, icon, checkable |
| `CheckBoxComponent` | Checkbox |
| `SwitchComponent` | Toggle switch |
| `ProgressBarComponent` | Progress bar |
| `CircularProgressComponent` | Circular progress indicator |
| `SliderComponent` | Slider |
| `SpinBoxComponent` | Number spin box |
| `ComboBoxComponent` | Combo box |
| `IconComponent` | Icon font character |
| `ImageComponent` | Image with rounded corners |
| `AvatarComponent` | Avatar with image or initials |
| `BadgeComponent` | Badge/tag |
| `RatingComponent` | Star rating |
| `SeparatorComponent` | Separator line |
| `CardComponent` | Card container |
| `SpacerComponent` | Spacer |
| `ExpandArrowComponent` | Expand/collapse arrow |

## Extending

### Creating Custom Components

Inherit from `AbstractComponent`:

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

        // Background circle
        p->setPen(QPen(QColor(200, 200, 200), 3));
        p->drawEllipse(r.center(), r.width()/2 - 3, r.height()/2 - 3);

        // Progress arc
        if (value > 0) {
            p->setPen(QPen(QColor(76, 175, 80), 3));
            int angle = value * 360 / 100 * 16;
            p->drawArc(r.adjusted(3, 3, -3, -3), 90 * 16, -angle);
        }

        // Percentage text
        p->drawText(r, Qt::AlignCenter, QString("%1%").arg(value));
    }

    void setValue(int v) { m_value = qBound(0, v, 100); update(); }
    int value() const { return m_value; }

private:
    int m_value = 0;
};
```

### Registering Custom Components

```cpp
class MyDelegate : public DelegateController
{
public:
    MyDelegate(QObject* parent = nullptr) : DelegateController(parent)
    {
        registerComponent<ProgressCircle>("progress");
        addItem<ProgressCircle>("progress", 40, Qt::AlignVCenter);

        bindTo("progress").property("value", ProgressRole);
    }
};
```

### Layout Item Types

```cpp
// WidgetItem - For visual components
auto widget = std::make_shared<WidgetItem>("id");
widget->setSizeHint(QSize(100, 30));
widget->setMinimumSize(QSize(50, 20));
widget->setMaximumSize(QSize(200, 50));
widget->setStretch(1);
widget->setAlignment(Qt::AlignVCenter);

// SpacerItem - For fixed spacing
auto spacer = std::make_shared<SpacerItem>(20, 10);

// BoxLayout - For nested layouts
auto innerLayout = std::make_shared<VBoxLayout>();
innerLayout->addItem(widget1);
innerLayout->addItem(widget2);
outerLayout->addItem(innerLayout);
```

## Layout Debugger

VLayout includes a visual debugger:

```cpp
#include <vlayout/debugger/sandbox_widget.h>

auto debugger = new VLayout::LayoutSandboxWidget();
debugger->setWindowTitle("Layout Debugger");
debugger->show();
```

**Features:**

- Visual preview with grid background
- Real-time parameter adjustment
- Add/Remove layout items
- Export/Import layout as JSON

## API Reference

### DelegateController

```cpp
// Components
template<typename T>
T* addItem(const QString& id, int width = -1, Qt::Alignment align = {});

// Layout
void setMargins(int left, int top, int right, int bottom);
void setSpacing(int spacing);
void setRowHeight(int height);
void addSpacing(int width);
void addStretch(int stretch = 1);

// Data Binding
BindingBuilder bindTo(const QString& componentId);

// Events
void onClick(const QString& id, ClickHandler handler);

// Utilities
static void toggleData(const QModelIndex& idx, int role);
static void setModelData(const QModelIndex& idx, const QVariant& value, int role);
```

### BindingBuilder

```cpp
bindTo("component")
    .display()                          // Qt::DisplayRole
    .text(int role)                     // Bind text
    .text(int role, Converter conv)     // With converter
    .font(const QFont& font)            // Fixed font
    .boldFont(int pointSize)            // Bold font
    .color(const QColor& color)         // Fixed color
    .color(int role, ColorConverter)    // Conditional color
    .property(const QString& name, int role)
    .visibleWhen(int role, Condition)
    .visibleWhenNotEmpty(int role)
    .checkedWhenTrue(int role)
    .onClick(ClickHandler);
```

## Project Structure

```text
VLayout/
├── src/vlayout/           # Core library
│   ├── framework.h        # Main include
│   ├── component.h        # Component interface
│   ├── components.h       # Built-in components
│   ├── binding.h          # Data binding
│   ├── delegatecontroller.h
│   ├── boxlayout.h        # Layout engine
│   ├── layoutitem.h       # Layout item base
│   ├── widgetitem.h       # Widget item
│   ├── spaceritem.h       # Spacer item
│   └── debugger/          # Debug tools
│
├── tests/                 # Unit tests
├── examples/              # Examples
└── docs/                  # Documentation
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

MIT License - see [LICENSE](LICENSE) file for details.

---

Made with ❤️ for Qt Developers
