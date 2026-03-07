# Sandbox 预览系统重构：使用真实 BoxLayout

**状态**: ✅ 已完成

## 背景

当前 `SandboxScene` 维护了一套独立的布局参数和 `SandboxItem` 数据结构，然后在 `computeLayout()` 中手动构建 `LayoutStruct` 并调用静态的 `BoxLayout::calculateGeometry()`。这存在以下问题：

1. API 重复（setSpacing, setMargins, addFixedItem 等）
2. 可能与真实 BoxLayout 行为产生不一致
3. 不支持嵌套布局
4. 维护成本高

## 目标

让 Sandbox 预览系统直接使用真实的 `BoxLayout` 实例，确保：
- 100% 布局行为一致性
- 支持嵌套布局
- 减少代码重复
- 简化维护

## 架构设计

### 核心变化

**之前架构**：
```
SandboxScene
├── m_spacing, m_leftMargin, ... (重复存储)
├── m_items: vector<SandboxItem> (独立数据结构)
└── computeLayout() → 手动构建 LayoutStruct → calculateGeometry()
```

**新架构**：
```
SandboxScene
├── m_layout: shared_ptr<BoxLayout> (真实布局实例)
│   └── 包含所有布局参数和子项
└── computeLayout() → m_layout->setGeometry() + activate()
                      ↓
              从 item->finalRect() 获取位置绘制
```

## 已完成的实现

### Phase 1: 准备工作 ✅
- [x] 1.1 为 LayoutItemGraphics 添加 setLayoutItem() 和 syncFromLayoutItem() 支持
- [x] 1.2 SandboxItem 保留作为显示缓存，从 LayoutItem 同步

### Phase 2: 核心重构 ✅
- [x] 2.1 SandboxScene 添加 `std::shared_ptr<BoxLayout> m_layout` 成员
- [x] 2.2 重写 computeLayout() 使用 `m_layout->activate()`
- [x] 2.3 重写 updateLayoutItems() 从 `finalRect()` 同步
- [x] 2.4 新增 generateDiagnostics() 从真实布局生成诊断信息

### Phase 3: API 迁移 ✅
- [x] 3.1 添加 `layout()` 访问器返回底层 BoxLayout
- [x] 3.2 保留旧 API（setItems, clearItems）作为兼容接口
- [x] 3.3 更新 SandboxPreview 代理方法

### Phase 4: 兼容性 ✅
- [x] 4.1 LayoutSandboxWidget 通过 setItems() 保持兼容
- [x] 4.2 旧代码无需修改即可工作

## 新增能力

### 直接操作真实布局

```cpp
auto preview = new SandboxPreview();
auto layout = preview->layout();  // 获取真实 BoxLayout

layout->clear();
layout->setContentsMargins(10, 10, 10, 10);
layout->setSpacing(8);
layout->addItem(createLabel("name", "Name:"));
layout->addItem(createStretch());
layout->addItem(createButton("btn", "OK"));

preview->computeLayout();  // 执行布局计算
```

### 支持嵌套布局（新功能）

```cpp
auto layout = preview->layout();

// 创建嵌套的水平布局
auto hLayout = std::make_shared<HBoxLayout>();
hLayout->addItem(createLabel("label1", "Name:"));
hLayout->addItem(createLineEdit("edit1"));
hLayout->addItem(createStretch());

// 添加到主布局（垂直）
layout->setDirection(BoxLayout::Direction::TopToBottom);
layout->addItem(hLayout);  // 直接嵌套！
layout->addItem(createButton("submit", "Submit"));

preview->computeLayout();
```

## 影响的文件

- `src/vlayout/debugger/layout_item_graphics.h` - 添加 LayoutItem 支持
- `src/vlayout/debugger/layout_item_graphics.cpp` - 实现 syncFromLayoutItem
- `src/vlayout/debugger/sandbox_scene.h` - 添加 m_layout 成员
- `src/vlayout/debugger/sandbox_scene.cpp` - 核心重构
- `src/vlayout/debugger/sandbox_preview.h` - 添加 layout() 访问器
- `src/vlayout/debugger/sandbox_preview.cpp` - 代理更新

## 优点

1. **一致性**：预览与实际布局行为 100% 一致
2. **功能完整**：支持嵌套布局等所有 BoxLayout 特性
3. **维护简单**：单一数据源，API 不重复
4. **自动更新**：BoxLayout 的改进自动生效
5. **向后兼容**：旧代码通过 setItems() 接口继续工作
