# Timeline Markdown 渲染设计文档

## 目标

为 Timeline 控件添加 Markdown 渲染和代码语法高亮功能。

## 技术选型

### Markdown 渲染
- **方案**: Qt 内置 QTextDocument::setMarkdown()
- **原因**: Qt 5.14+ 原生支持，无需外部依赖
- **限制**: 不支持复杂扩展（表格支持有限）

### 代码高亮
- **方案**: KSyntaxHighlighting (KDE Frameworks)
- **原因**:
  - 只依赖 Qt (QtCore)，无需完整 KDE
  - 支持 300+ 语言
  - 可自定义主题
- **仓库**: https://invent.kde.org/frameworks/syntax-highlighting

## 架构设计

### 文件结构

```
examples/timeline/
├── markdown_renderer.h      # Markdown 渲染器
├── markdown_renderer.cpp
├── code_highlighter.h       # 代码高亮器
├── code_highlighter.cpp
└── timeline_components.cpp  # 更新 MessageContentComponent
```

### 类设计

#### 1. MarkdownRenderer

```cpp
class MarkdownRenderer {
public:
    // 解析 Markdown 并返回格式化信息
    struct Block {
        enum Type { Text, Code, List, Header };
        Type type;
        QString content;
        QString language;  // 仅代码块
    };

    static QVector<Block> parse(const QString& markdown);

    // 渲染到 QTextDocument
    static void render(QTextDocument* doc, const QString& markdown);
};
```

#### 2. CodeHighlighter

```cpp
class CodeHighlighter : public QSyntaxHighlighter {
public:
    CodeHighlighter(QTextDocument* parent, const QString& language);

protected:
    void highlightBlock(const QString& text) override;

private:
    KSyntaxHighlighting::Repository* m_repo;
    KSyntaxHighlighting::Highlighter* m_highlighter;
};
```

### 渲染流程

```
Markdown 文本
    ↓
MarkdownRenderer::parse()
    ↓
┌─────────────────────────────────┐
│ Block 1: Text (普通段落)         │
│ Block 2: Code (代码块, lang=cpp) │
│ Block 3: Text (普通段落)         │
└─────────────────────────────────┘
    ↓
MessageContentComponent::paint()
    ↓
├─ Text Block  → QTextDocument::drawContents()
└─ Code Block  → CodeHighlighter + 自定义绘制
```

## 实现阶段

### Phase 1: 基础 Markdown 渲染
1. 创建 MarkdownRenderer 类
2. 支持: 标题、粗体、斜体、链接、列表
3. 集成到 MessageContentComponent

### Phase 2: 代码高亮
1. 添加 KSyntaxHighlighting 依赖
2. 创建 CodeHighlighter 类
3. 支持: 关键词、字符串、注释着色

### Phase 3: 优化
1. 渲染缓存
2. 高度预计算优化
3. 主题适配

## 依赖管理

### .pro 文件更新

```qmake
# timeline.pro
# KSyntaxHighlighting (需要安装 KF5SyntaxHighlighting)
unix {
    packagesExist(KF5SyntaxHighlighting) {
        PKGCONFIG += KF5SyntaxHighlighting
        DEFINES += HAS_KSYNTAX_HIGHLIGHTING
    }
}

# 或手动指定路径
# INCLUDEPATH += /path/to/kf5/include
# LIBS += -lKF5SyntaxHighlighting
```

## 主题集成

代码高亮主题需要与 Timeline 深色主题一致：

```cpp
namespace Theme {
// 代码高亮色 (对应 KSyntaxHighlighting 主题)
constexpr QColor codeKeyword{86, 156, 214};   // 关键词
constexpr QColor codeString{206, 145, 120};   // 字符串
constexpr QColor codeComment{106, 153, 85};   // 注释
constexpr QColor codeNumber{181, 206, 168};   // 数字
}
```

## 性能考虑

1. **QTextDocument 缓存**: 为每个消息缓存解析后的 QTextDocument
2. **延迟渲染**: 只渲染可见区域
3. **代码高亮缓存**: 避免重复解析语法定义

## 测试用例

1. 纯文本消息
2. 带格式的 Markdown 消息（粗体、链接）
3. 内联代码 `code`
4. 代码块（多种语言）
5. 嵌套列表
6. 超长内容性能测试
