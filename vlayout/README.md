# VLayout

Lightweight virtual layout framework with Qt-compatible algorithms.

## Features

- **Qt-Compatible Algorithm** - Uses the exact same geometry calculation as Qt's QBoxLayout
- **No Widget Dependency** - Only requires Qt Core, not Widgets
- **Full Constraint Support** - minimumSize, maximumSize, stretch factors, alignment
- **Modern C++** - C++11 smart pointers, enum class, type safety
- **Professional Quality** - Doxygen documentation, consistent API

## Quick Start

### Integration

Copy the `vlayout` folder to your project and add to your .pro file:

```qmake
include(vlayout/vlayout.pri)
```

### Basic Usage

```cpp
#include <VLayout>

// Create horizontal layout
auto layout = std::make_shared<VLayout::HBoxLayout>();
layout->setContentsMargins(10, 10, 10, 10);
layout->setSpacing(8);

// Add label
auto label = VLayout::createLabel("nameLabel", "Name:");
label->setMinimumWidth(60);
label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
layout->addItem(label);

// Add expanding spacer
layout->addItem(VLayout::createExpandingSpacer());

// Add button
auto button = VLayout::createButton("okBtn", "OK");
button->setFixedSize(QSize(80, 28));
layout->addItem(button);

// Activate layout
layout->setGeometry(QRect(0, 0, 300, 50));
layout->activate();

// Use finalRect() for painting
QRect labelRect = label->finalRect();
QRect buttonRect = button->finalRect();
```

## API Overview

### Layout Classes

| Class | Description |
|-------|-------------|
| `BoxLayout` | Horizontal or vertical box layout |
| `HBoxLayout` | Convenience class for horizontal layout |
| `VBoxLayout` | Convenience class for vertical layout |

### Item Classes

| Class | Description |
|-------|-------------|
| `WidgetItem` | Virtual widget item |
| `SpacerItem` | Fixed or expanding spacer |

### Factory Functions

```cpp
// Widgets
auto label = VLayout::createLabel("id", "Text");
auto button = VLayout::createButton("id", "Text");
auto checkbox = VLayout::createCheckBox("id", "Text");
auto lineEdit = VLayout::createLineEdit("id", "Text");
auto progressBar = VLayout::createProgressBar("id", 50);
auto slider = VLayout::createSlider("id", 0);
auto spinBox = VLayout::createSpinBox("id", 0);
auto comboBox = VLayout::createComboBox("id");

// Spacers
auto fixed = VLayout::createFixedSpacer(20, true);      // 20px horizontal
auto expanding = VLayout::createExpandingSpacer(1);     // stretch=1
```

## Layout Algorithm

The algorithm works in three scenarios:

1. **Space < minimumSize** - Items shrunk proportionally from largest first
2. **minimumSize <= Space < sizeHint** - Equal distribution respecting minimums
3. **Space >= sizeHint** - Distribution by stretch factors respecting maximums

This is a direct port of Qt's `qGeomCalc` function.

## License

Same as your Qt installation.
