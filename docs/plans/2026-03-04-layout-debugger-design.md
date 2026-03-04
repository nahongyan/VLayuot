# VLayout 调试器设计文档

## 概述

一个轻量级的布局调试工具，用于开发时可视化 VLayout 布局计算过程，帮助诊断布局问题。

## 需求总结

| 方面 | 选择 |
|------|------|
| 用途 | 开发时调试，离线分析 + 问题诊断 |
| 形式 | Qt 窗口 |
| 入口 | 静态沙盒模式，输入参数实时预览 |
| 界面 | 上参数下预览 |
| 预览 | 矩形框 + 网格背景 + 标尺 |

## 整体架构

### 文件结构

```
src/vlayout/debugger/
├── layout_debugger.h       # 主调试器类
├── layout_debugger.cpp
├── sandbox_widget.h        # 沙盒窗口控件
├── sandbox_widget.cpp
├── sandbox_preview.h       # 预览区（绘制布局）
├── sandbox_preview.cpp
└── debugger.pri            # qmake 包含文件
```

### 核心类关系

```
LayoutSandboxWidget (QWidget)
├── m_paramPanel (QWidget)      // 上方参数面板
│   ├── 容器尺寸输入
│   ├── spacing/margins 输入
│   └── 添加/删除布局项按钮
│
├── m_splitter (QSplitter)      // 分割器
│
├── m_preview (SandboxPreview)  // 左侧预览区（网格+布局矩形）
│
└── m_resultTable (QTableWidget) // 右侧计算结果表格
```

## 参数面板设计

### 布局

```
┌─────────────────────────────────────────────────────────────┐
│ 【容器设置】                                                 │
│   宽度: [____400____] px    高度: [____300____] px          │
│   Direction: [水平 ▼]                                       │
│                                                             │
│ 【布局设置】                                                 │
│   Spacing: [____8____] px                                   │
│   Margins: 左[8] 上[8] 右[8] 下[8]                          │
│                                                             │
│ 【布局项列表】                                    [+] [-]   │
│ ┌─────────────────────────────────────────────────────────┐│
│ │ ID        │ Type   │ SizeHint │ Stretch │ Min │ Max    ││
│ ├───────────┼────────┼──────────┼─────────┼─────┼────────┤│
│ │ header    │ Fixed  │    40    │    0    │  0  │ 40     ││
│ │ content   │ Stretch│   100    │    1    │  0  │ 9999   ││
│ │ footer    │ Fixed  │    30    │    0    │  0  │ 30     ││
│ └─────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────┘
```

### 布局项类型

| Type | 含义 | 限制 |
|------|------|------|
| Fixed | 固定尺寸 | min=max=sizeHint |
| Stretch | 弹性填充 | stretch=1, max=无限 |
| Spacer | 间隔 | 固定尺寸，不绘制 |
| Widget | 模拟控件 | 可设置任意参数 |

### 交互

- 修改任意参数 → 实时重新计算布局 → 预览区立即刷新
- 双击表格行 → 弹出详细编辑对话框
- `[+]` 按钮 → 添加新布局项（弹出选择类型）
- `[-]` 按钮 → 删除选中项

## 预览区设计

### SandboxPreview 控件

```
┌────────────────────────────────────────────────────────┐
│  0    50   100  150  200  250  300  350  400    ← 标尺 │
│  ┌─────────────────────────────────────────────┐      │
│  │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│      │
│  │░░┌────────────────────────────────────────┐░│      │
│  │░░│            header (40px)               │░│      │
│  │░░└────────────────────────────────────────┘░│      │
│  │░░                                         ░│      │
│  │░░┌────────────────────────────────────────┐░│      │
│  │░░│                                       │░│      │
│  │░░│          content (stretch=1)          │░│      │
│  │░░│            final: 222px               │░│      │
│  │░░│                                       │░│      │
│  │░░└────────────────────────────────────────┘░│      │
│  │░░                                         ░│      │
│  │░░┌────────────────────────────────────────┐░│      │
│  │░░│            footer (30px)               │░│      │
│  │░░└────────────────────────────────────────┘░│      │
│  │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│      │
│  └─────────────────────────────────────────────┘      │
│  ↑ 网格背景 (每格 10px)                                │
└────────────────────────────────────────────────────────┘
```

### 绘制元素

| 元素 | 样式 |
|------|------|
| 网格背景 | 浅灰色虚线，每 10px 一格，每 50px 加粗 |
| 标尺 | 顶部和左侧，显示像素刻度 |
| 布局矩形 | 彩色填充 + 边框，不同类型用不同颜色 |
| 组件 ID | 居中显示，字体清晰 |
| 尺寸标注 | 在矩形内部或旁边显示 finalRect |

### 颜色方案

```cpp
// 按类型着色
QColor FixedColor   = QColor(76, 175, 80, 100);   // 绿色
QColor StretchColor = QColor(33, 150, 243, 100);  // 蓝色
QColor SpacerColor  = QColor(158, 158, 158, 100); // 灰色
QColor WidgetColor  = QColor(255, 152, 0, 100);   // 橙色
```

### 交互

- 鼠标悬停 → 高亮该项，显示 tooltip（详细尺寸信息）
- 鼠标点击 → 选中该项，右侧表格同步高亮对应行
- 滚轮 → 缩放预览（可选功能）

## 计算结果表格

### 表格结构

```
┌─────────────────────────────────────────────────────────────────────────┐
│ 【布局计算结果】                                                         │
│ 容器可用空间: 384px (400 - margins 8*2)                                 │
│ 项总需求: 170px (sizeHint) | 最小需求: 70px | 剩余分配: 214px            │
├─────────────────────────────────────────────────────────────────────────┤
│ ┌───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┐     │
│ │ ID    │ Hint  │ Min   │ Max   │Stretch│  Pos  │ Size  │ 状态  │     │
│ ├───────┼───────┼───────┼───────┼───────┼───────┼───────┼───────┤     │
│ │header │  40   │   0   │  40   │   0   │   8   │  40   │ ✓固定 │     │
│ │spacing│   -   │   -   │   -   │   -   │  48   │   8   │ 间隔  │     │
│ │content│ 100   │   0   │ 9999  │   1   │  56   │  214  │ ✓拉伸 │     │
│ │spacing│   -   │   -   │   -   │   -   │ 270   │   8   │ 间隔  │     │
│ │footer │  30   │   0   │  30   │   0   │ 278   │  30   │ ✓固定 │     │
│ └───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┘     │
└─────────────────────────────────────────────────────────────────────────┘
```

### 列说明

| 列 | 含义 |
|----|------|
| ID | 布局项标识 |
| Hint | sizeHint 值 |
| Min | minimumSize |
| Max | maximumSize |
| Stretch | 拉伸因子 |
| Pos | 计算后的起始位置 |
| Size | 计算后的实际尺寸 |
| 状态 | 正常/警告/错误 |

### 状态标识

| 状态 | 条件 | 颜色 |
|------|------|------|
| ✓固定 | max=min=hint | 绿色 |
| ✓拉伸 | stretch>0 且正常分配 | 蓝色 |
| ⚠压缩 | size < sizeHint | 橙色 |
| ✗溢出 | size < minimumSize | 红色 |

### 诊断提示

表格下方显示诊断信息：

```
┌─────────────────────────────────────────────────┐
│ 💡 诊断:                                        │
│   • content 获得了全部剩余空间 (214px)          │
│   • 布局空间充足，所有项按预期分配              │
└─────────────────────────────────────────────────┘
```

## API 设计

### LayoutSandboxWidget 公共接口

```cpp
namespace VLayout {

class LayoutSandboxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LayoutSandboxWidget(QWidget* parent = nullptr);
    ~LayoutSandboxWidget();

    // ========== 容器设置 ==========

    void setContainerSize(int width, int height);
    QSize containerSize() const;

    void setLayoutDirection(BoxLayout::Direction direction);

    // ========== 布局参数 ==========

    void setSpacing(int spacing);
    void setMargins(int left, int top, int right, int bottom);
    void setMargins(int uniform);

    // ========== 布局项管理 ==========

    /// 添加固定尺寸项
    void addFixedItem(const QString& id, int size);

    /// 添加弹性项
    void addStretchItem(const QString& id, int stretch = 1,
                        int sizeHint = 100, int minSize = 0);

    /// 添加间隔
    void addSpacing(int spacing);

    /// 添加自定义项
    void addCustomItem(const QString& id,
                       int sizeHint, int minSize, int maxSize,
                       int stretch = 0);

    /// 清空所有项
    void clearItems();

    /// 删除指定项
    void removeItem(const QString& id);

    // ========== 从现有布局加载 ==========

    /// 从 BoxLayout 加载（会深拷贝布局项）
    void loadLayout(std::shared_ptr<BoxLayout> layout);

    // ========== 导出 ==========

    /// 导出为 JSON 配置
    QString exportToJson() const;

    /// 从 JSON 加载
    void loadFromJson(const QString& json);

signals:
    /// 布局计算完成时发出
    void layoutComputed(const QString& summary);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace VLayout
```

### 使用示例

```cpp
// 示例1：快速测试一个布局
auto sandbox = new VLayout::LayoutSandboxWidget();
sandbox->setContainerSize(400, 200);
sandbox->setMargins(16, 8, 16, 8);
sandbox->setSpacing(8);
sandbox->addFixedItem("icon", 36);
sandbox->addStretchItem("title", 1);
sandbox->addFixedItem("badge", 24);
sandbox->show();

// 示例2：调试现有布局
auto layout = std::make_shared<VLayout::HBoxLayout>();
// ... 配置布局 ...
auto sandbox = new VLayout::LayoutSandboxWidget();
sandbox->loadLayout(layout);
sandbox->show();
```

## 集成与条件编译

### 项目集成

```qmake
# vlayout.pri 中添加
DEFINES += VLAYOUT_DEBUGGER  # 启用调试器

# 条件包含调试器模块
contains(DEFINES, VLAYOUT_DEBUGGER) {
    include(debugger/debugger.pri)
}
```

### debugger.pri 内容

```qmake
HEADERS += \
    $$PWD/layout_debugger.h \
    $$PWD/sandbox_widget.h \
    $$PWD/sandbox_preview.h

SOURCES += \
    $$PWD/layout_debugger.cpp \
    $$PWD/sandbox_widget.cpp \
    $$PWD/sandbox_preview.cpp
```

### 条件编译使用方式

```cpp
// 在代码中使用
#ifdef VLAYOUT_DEBUGGER
#include <vlayout/debugger.h>
#endif

void MyDelegate::paint(QPainter* painter, ...)
{
    // ... 布局计算 ...

#ifdef VLAYOUT_DEBUGGER
    // 调试时打开
    if (qApp->keyboardModifiers() & Qt::ControlModifier) {
        auto sandbox = new VLayout::LayoutSandboxWidget();
        sandbox->loadLayout(m_layout);
        sandbox->setAttribute(Qt::WA_DeleteOnClose);
        sandbox->show();
    }
#endif
}
```

### Release 构建自动移除

```qmake
# Release 配置中自动禁用
CONFIG(release, debug|release) {
    DEFINES -= VLAYOUT_DEBUGGER
}
```

## 实现优先级

1. **P0 - 核心功能**
   - SandboxPreview 绘制（网格、标尺、布局矩形）
   - 参数面板基础控件
   - 布局计算结果显示

2. **P1 - 交互增强**
   - 实时参数修改响应
   - 表格与预览联动高亮
   - 诊断信息显示

3. **P2 - 扩展功能**
   - 从现有布局加载
   - JSON 导入导出
   - 嵌套布局支持
