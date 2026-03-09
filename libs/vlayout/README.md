# VLayout Framework

A declarative delegate framework for Qt Model/View.

## Features

- **Declarative API** - Describe UI layout and data binding declaratively
- **Component-based Architecture** - Reusable UI components with custom extension support
- **Fluent Data Binding** - Chain API to bind Model data to component properties
- **Event Handling** - Declarative click event handling
- **Layout Engine** - QBoxLayout-style H/V layouts with caching optimization
- **Rich Component Library** - 20+ built-in components
- **Zero Dependency** - Only requires Qt Core and Qt Gui
- **High Performance** - Layout result caching with fast path optimization

## Quick Start

### Installation

**Option 1: Link Prebuilt Library**

```qmake
include(libs/vlayout/vlayout.pri)
```

**Option 2: Compile Source Directly**

```qmake
include(libs/vlayout/vlayout_source.pri)
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
        // Add components
        addItem<LabelComponent>("title", -1);  // Stretch fill
        addItem<ButtonComponent>("btn", 60);    // Fixed 60px
        addSpacing(8);
        setRowHeight(36);

        // Data binding
        bindTo("title")
            .text(Qt::DisplayRole)
            .color(QColor(232, 232, 240));

        // Event handling
        onClick("btn", [](const QModelIndex& idx, IComponent*) {
            qDebug() << "Clicked:" << idx.data().toString();
        });
    }
};
```

## Core Concepts

### DelegateController

The core class inheriting from `QStyledItemDelegate`:

- **Component Management** - `addComponent()`, `addItem<T>()`
- **Layout Description** - `setLayout()`, `setRow()`, `addItem()`, `addSpacing()`
- **Data Binding** - `bindTo()`
- **Event Handling** - `onClick()`

### Components

Components are drawable UI elements. Create custom components by inheriting from `AbstractComponent`:

```cpp
class MyComponent : public AbstractComponent
{
public:
    MyComponent(const QString& id) : AbstractComponent(id) {}
    QString type() const override { return "MyComponent"; }

    void paint(ComponentContext& ctx) override {
        ctx.painter->drawText(geometry(), Qt::AlignCenter, text());
    }

    void setText(const QString& t) { setProperty("text", t); }
    QString text() const { return property("text").toString(); }
};
```

### Data Binding

```cpp
bindTo("title")
    .text(Qt::DisplayRole)
    .boldFont(12)
    .color(QColor(255, 255, 255));

bindTo("badge")
    .text(StatusRole, statusConverter)
    .color(StatusRole, colorConverter)
    .visibleWhenNotEmpty(StatusRole);

bindTo("pin")
    .checkedWhenTrue(PinnedRole)
    .onClick([](const QModelIndex& idx, IComponent*) {
        toggleData(idx, PinnedRole);
    });
```

### Layout

```cpp
// Simple row layout (QBoxLayout style)
setMargins(16, 8, 16, 8);
setSpacing(12);

addItem<SpacerComponent>("indent");
addItem<ExpandArrowComponent>("arrow", 16);  // Fixed 16px
addItem<LabelComponent>("name", -1);         // Stretch
addItem<BadgeComponent>("count", 24);
addSpacing(8);

setRowHeight(36);
```

## Built-in Components

| Component | Description |
|-----------|-------------|
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
| `ImageComponent` | Image with rounded corners support |
| `AvatarComponent` | Avatar with image or initials |
| `BadgeComponent` | Badge/tag |
| `RatingComponent` | Star rating |
| `SeparatorComponent` | Separator line |
| `CardComponent` | Card container |
| `SpacerComponent` | Spacer |
| `ExpandArrowComponent` | Expand/collapse arrow |

## Example

See `demo/vs_recent/` for a complete Visual Studio-style recent projects list demo.

## License

MIT License
