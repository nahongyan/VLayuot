# AI 编程助手时间轴组件设计方案

> 基于市面上 20+ 款主流 AI 编程助手产品的 UI 分析

## 一、市场产品概览

### 1.1 国际产品

| # | 产品 | 类型 | 价格 | 核心特点 | UI 模式 |
|---|------|------|------|----------|---------|
| 1 | **Cursor** | AI IDE | $20/月 | Composer 多文件编辑，Agent 模式 | 侧边栏 + 内联 |
| 2 | **GitHub Copilot** | VSCode 插件 | $10/月 | 行业标准，企业合规 | 侧边栏 + 内联 + Workspace |
| 3 | **Claude Code** | CLI + 插件 | $20/月 | 终端原生，Agent 自主执行 | 终端 TUI + VSCode 侧边栏 |
| 4 | **Windsurf** | AI IDE | $15/月 | Cascade Flow，远程索引 | 侧边栏 + 内联 |
| 5 | **Cline** | VSCode 插件 | 开源 | 自主编码 Agent，75+ 模型 | 侧边栏 + Diff 视图 |
| 6 | **Continue** | VSCode 插件 | 开源 | BYOK，本地模型支持 | 侧边栏 + 内联 |
| 7 | **Codeium** | VSCode 插件 | 免费 | 70+ 语言，无限补全 | 侧边栏 + 内联 |
| 8 | **Cody (Sourcegraph)** | VSCode 插件 | $9/月 | 代码图谱搜索，大型代码库 | 侧边栏 |
| 9 | **Tabnine** | VSCode 插件 | $12/月 | 隐私优先，本地部署 | 内联 + 侧边栏 |
| 10 | **Amazon Q Developer** | VSCode 插件 | 免费 | AWS 集成，安全扫描 | 侧边栏 + 内联 |
| 11 | **Zed** | 编辑器 | 免费/$20 Pro | Rust 高性能，多模型 | 侧边栏 + 内联 Assist |
| 12 | **Aider** | CLI 工具 | 开源 | 终端 AI，Git 深度集成 | 终端 TUI |
| 13 | **Replit Agent** | 云端 IDE | 免费/付费 | 浏览器内完整开发环境 | Web 聊天界面 |
| 14 | **Bolt.new** | Web 生成器 | $20/月 | 全栈应用一键生成 | Web 聊天界面 |
| 15 | **v0 (Vercel)** | Web 生成器 | 免费/付费 | 前端 UI 生成专家 | Web 聊天界面 |
| 16 | **JetBrains AI** | IDE 插件 | 订阅制 | 深度 IDE 集成，JVM 优化 | 侧边栏 + 内联 |
| 17 | **Gemini Code Assist** | VSCode 插件 | 免费 | Gemini 2.5，180K 补全/月 | 侧边栏 + 内联 |

### 1.2 国内产品

| # | 产品 | 公司 | 类型 | 核心特点 |
|---|------|------|------|----------|
| 18 | **Trae** | 字节跳动 | AI IDE | 中文优化，Builder 模式，免费 |
| 19 | **通义灵码** | 阿里云 | VSCode 插件 | 千问 3，中文注释理解，MCP 集成 |
| 20 | **百度 Comate** | 百度 | VSCode 插件 | 文心大模型，企业版支持 |
| 21 | **iFlyCode** | 科大讯飞 | VSCode 插件 | 星火大模型，SQL 优化 |
| 22 | **CodeGeeX** | 清华 | VSCode 插件 | 多语言开源模型 |

---

## 二、UI 组件模式分析

### 2.1 三种主要交互模式

```
┌─────────────────────────────────────────────────────────────┐
│                     交互模式分类                              │
├─────────────────────────────────────────────────────────────┤
│  1. 侧边栏聊天 (Sidebar Chat)                                │
│     ├── VSCode Copilot Chat                                 │
│     ├── Cursor Chat                                         │
│     ├── Windsurf Cascade                                    │
│     └── 特点：多轮对话、上下文引用、历史记录                    │
├─────────────────────────────────────────────────────────────┤
│  2. 内联辅助 (Inline Assist)                                 │
│     ├── VSCode Copilot Inline (Ctrl+I)                      │
│     ├── Cursor Composer (Cmd+I)                             │
│     ├── Zed Inline Assist (Ctrl+Enter)                      │
│     └── 特点：嵌入编辑器、Diff 预览、快速应用                  │
├─────────────────────────────────────────────────────────────┤
│  3. 终端界面 (Terminal TUI)                                  │
│     ├── Claude Code CLI                                     │
│     ├── Aider                                               │
│     └── 特点：纯文本、Diff 格式、Git 集成                      │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 消息/节点类型汇总

基于 20+ 产品的分析，AI 编程助手的时间轴需要支持以下节点类型：

| 类型 | 英文名 | 出现频率 | 描述 |
|------|--------|----------|------|
| **用户消息** | UserMessage | 100% | 用户输入的文本/问题 |
| **AI 文本回复** | AIMessage | 100% | AI 的文本回答 |
| **代码块** | CodeBlock | 100% | 带语法高亮的代码 |
| **工具调用** | ToolCall | 90% | 文件读取、命令执行等 |
| **工具结果** | ToolResult | 90% | 工具执行的返回结果 |
| **思考过程** | ThinkingStep | 60% | AI 推理过程（Claude/Cursor） |
| **文件变更** | FileChange | 80% | Diff 格式的代码变更 |
| **错误信息** | ErrorMessage | 70% | 执行失败或错误提示 |
| **系统消息** | SystemMessage | 50% | 系统提示、状态更新 |
| **图片/附件** | Attachment | 40% | 上传的截图、设计稿 |

### 2.3 核心 UI 组件清单

```
TimelineWidget (主容器)
├── TimelineHeader (头部)
│   ├── 模型选择器
│   ├── 模式切换 (Ask/Edit/Agent)
│   └── 操作按钮 (清空/导出/设置)
│
├── TimelineView (时间轴视图)
│   ├── MessageDelegate (消息代理)
│   │   ├── UserBubble (用户气泡 - 右对齐)
│   │   └── AIBubble (AI 气泡 - 左对齐)
│   │
│   ├── CodeBlockDelegate (代码块代理)
│   │   ├── 语言标签
│   │   ├── 语法高亮区
│   │   ├── 复制按钮
│   │   └── 应用/插入按钮
│   │
│   ├── ToolCallDelegate (工具调用代理)
│   │   ├── 工具图标 + 名称
│   │   ├── 参数折叠面板
│   │   ├── 执行状态指示器
│   │   └── 结果展示区
│   │
│   ├── ThinkingDelegate (思考过程代理)
│   │   ├── 折叠/展开控制
│   │   └── 步骤列表
│   │
│   └── FileChangeDelegate (文件变更代理)
│       ├── 文件路径
│       ├── Diff 视图 (+/- 行)
│       └── 接受/拒绝按钮
│
└── TimelineInput (输入区域)
    ├── 上下文引用区 (@file, @folder)
    ├── 多行文本输入
    ├── 附件按钮
    └── 发送按钮
```

---

## 三、数据模型设计

### 3.1 核心数据结构

```cpp
// 节点类型枚举
enum class TimelineNodeType {
    UserMessage,        // 用户消息
    AIMessage,          // AI 回复
    CodeBlock,          // 代码块
    ToolCall,           // 工具调用
    ToolResult,         // 工具结果
    ThinkingStep,       // 思考步骤
    FileChange,         // 文件变更
    ErrorMessage,       // 错误消息
    SystemMessage,      // 系统消息
    Attachment          // 附件
};

// 工具调用状态
enum class ToolCallStatus {
    Pending,            // 等待执行
    Running,            // 执行中
    Success,            // 成功
    Error,              // 失败
    Cancelled           // 已取消
};

// 消息部分（Parts-based 架构，参考 assistant-ui）
struct TimelinePart {
    QString id;
    TimelineNodeType type;

    // 文本内容
    QString text;

    // 代码块专用
    QString language;
    QString code;

    // 工具调用专用
    QString toolName;
    QVariantMap toolArgs;
    QVariantMap toolResult;
    ToolCallStatus toolStatus = ToolCallStatus::Pending;

    // 文件变更专用
    QString filePath;
    QString oldContent;
    QString newContent;

    // 通用元数据
    QVariantMap metadata;
};

// 时间轴节点
struct TimelineNode {
    QString id;
    TimelineNodeType type;
    QList<TimelinePart> parts;

    // 角色区分
    enum Role { User, Assistant, System, Tool } role;

    // 状态
    bool isStreaming = false;      // 是否流式输出中
    bool isExpanded = true;        // 是否展开
    QDateTime timestamp;

    // 元数据
    QString modelName;             // 使用的模型
    int tokenCount = 0;            // Token 数量
    QVariantMap extra;
};
```

### 3.2 Model 接口

```cpp
class TimelineModel : public QAbstractListModel {
    Q_OBJECT

public:
    // 角色定义
    enum Roles {
        NodeIdRole = Qt::UserRole + 1,
        NodeTypeRole,
        NodeRole,           // User/Assistant/System/Tool
        PartsRole,          // QList<TimelinePart>
        IsStreamingRole,
        IsExpandedRole,
        TimestampRole,
        ModelNameRole,
        TokenCountRole
    };

    // 核心方法
    void addNode(const TimelineNode& node);
    void updateNode(const QString& id, const TimelineNode& node);
    void appendPart(const QString& nodeId, const TimelinePart& part);
    void updateStreamingText(const QString& nodeId, const QString& text);

    // 便捷方法
    void addUserMessage(const QString& text);
    void addAIMessage(const QString& text, bool streaming = false);
    void addCodeBlock(const QString& language, const QString& code);
    void addToolCall(const QString& toolName, const QVariantMap& args);
    void addToolResult(const QString& toolName, const QVariantMap& result);
    void addFileChange(const QString& path, const QString& diff);
};
```

---

## 四、Delegate 设计

### 4.1 Delegate 类型规划

基于 VLayout 框架，需要实现以下 Delegate：

| Delegate | 基类 | 说明 |
|----------|------|------|
| `UserMessageDelegate` | DelegateController | 用户消息气泡 |
| `AIMessageDelegate` | DelegateController | AI 消息气泡（支持流式） |
| `CodeBlockDelegate` | DelegateController | 代码块 + 语法高亮 |
| `ToolCallDelegate` | DelegateController | 工具调用卡片 |
| `ThinkingDelegate` | DelegateController | 思考过程折叠面板 |
| `FileChangeDelegate` | DelegateController | Diff 视图 |

### 4.2 统一 Delegate 方案

```cpp
class TimelineDelegate : public DelegateController {
public:
    TimelineDelegate(QObject* parent = nullptr);

protected:
    // 根据节点类型切换布局
    void setupLayout(const QModelIndex& index);

private:
    // 子布局
    void setupUserMessage(const QString& text);
    void setupAIMessage(const QString& text, bool streaming);
    void setupCodeBlock(const QString& language, const QString& code);
    void setupToolCall(const QString& name, const QVariantMap& args,
                       ToolCallStatus status);
    void setupThinking(const QList<QString>& steps);
    void setupFileChange(const QString& path, const QString& diff);
};
```

### 4.3 新增组件需求

需要在 VLayout 中新增以下组件：

| 组件 | 说明 | 优先级 |
|------|------|--------|
| `SyntaxHighlightComponent` | 代码语法高亮 | P0 |
| `CopyButtonComponent` | 复制到剪贴板按钮 | P0 |
| `ExpandableCardComponent` | 可折叠卡片 | P1 |
| `DiffViewComponent` | Diff 对比视图 | P1 |
| `StatusIndicatorComponent` | 状态指示器（加载/成功/失败） | P1 |
| `StreamingTextComponent` | 流式文本（打字机效果） | P2 |
| `MarkdownComponent` | Markdown 渲染 | P2 |

---

## 五、关键特性实现

### 5.1 流式输出

```cpp
// 流式文本更新
void TimelineDelegate::onStreamUpdate(const QString& nodeId,
                                       const QString& chunk) {
    m_streamingBuffer[nodeId] += chunk;

    // 触发局部重绘
    QModelIndex idx = m_model->indexForNodeId(nodeId);
    emit dataChanged(idx, idx, {TextRole});
}
```

### 5.2 代码块语法高亮

```cpp
class SyntaxHighlightComponent : public AbstractComponent {
public:
    void setLanguage(const QString& lang);
    void setCode(const QString& code);

    void paint(ComponentContext& ctx) override {
        // 使用 QSyntaxHighlighter 或第三方库
        QSyntaxHighlighter* highlighter = getHighlighter(m_language);
        highlighter->setDocument(m_document);
        // ... 渲染高亮后的文本
    }

private:
    QString m_language;
    QString m_code;
    QTextDocument m_document;
};
```

### 5.3 工具调用可视化

```cpp
void ToolCallDelegate::setupLayout(const QString& name,
                                    const QVariantMap& args,
                                    ToolCallStatus status) {
    // 头部：图标 + 工具名 + 状态
    addItem<IconComponent>("icon", 20);
    addItem<LabelComponent>("name", -1);
    addItem<StatusIndicatorComponent>("status", 24);

    // 可折叠的参数面板
    addItem<ExpandableCardComponent>("args-card");
    bindTo("args-card").visibleWhen(true);  // 默认折叠

    // 参数 JSON
    auto* argsLabel = addNestedItem<LabelComponent>("args-card", "args");
    argsLabel->setText(formatJson(args));
}
```

### 5.4 文件变更 Diff

```cpp
void FileChangeDelegate::setupLayout(const QString& path,
                                      const QString& diff) {
    // 文件路径
    addItem<IconComponent>("file-icon", 16);
    addItem<LabelComponent>("file-path", -1);

    // Diff 视图
    addItem<DiffViewComponent>("diff", -1);
    bindTo("diff").diffContent(diff);

    // 操作按钮
    addItem<ButtonComponent>("accept", 60);
    addItem<ButtonComponent>("reject", 60);

    onClick("accept", [](const QModelIndex& idx, IComponent*) {
        // 应用变更
    });
}
```

---

## 六、交互设计

### 6.1 键盘快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+L` | 打开/聚焦时间轴 |
| `Ctrl+I` | 内联编辑模式 |
| `Ctrl+Enter` | 发送消息 |
| `Ctrl+Shift+C` | 复制最后代码块 |
| `Escape` | 取消流式输出 |
| `Ctrl+Z` | 撤销最后一次变更 |

### 6.2 上下文引用

```
支持 @ 提及：
├── @file:path/to/file.cpp    引用特定文件
├── @folder:src/              引用整个目录
├── @symbol:ClassName         引用符号
├── @selection                引用当前选中
├── @editor                   引用当前编辑器
└── @image                    上传图片
```

### 6.3 状态管理

```cpp
// 模式切换
enum class TimelineMode {
    Ask,        // 问答模式：只回答，不修改代码
    Edit,       // 编辑模式：建议修改，需确认
    Agent       // 代理模式：自主执行任务
};

// 检查点机制
struct Checkpoint {
    QString id;
    QDateTime timestamp;
    QList<FileState> fileStates;
    QString description;
};
```

---

## 七、技术选型建议

### 7.1 语法高亮

| 方案 | 优点 | 缺点 |
|------|------|------|
| QSyntaxHighlighter | Qt 原生，无依赖 | 需要自己定义规则 |
| highlight.js (via QTextDocument) | 语言支持丰富 | 需要 JavaScript 引擎 |
| 自定义词法分析 | 性能最优 | 开发量大 |

**推荐**：QSyntaxHighlighter + 预定义规则（优先支持 C++/Python/JavaScript）

### 7.2 Markdown 渲染

| 方案 | 优点 | 缺点 |
|------|------|------|
| QTextDocument + 简单解析 | 轻量 | 功能有限 |
| cmark-gfm | 完整 GitHub 风格 | C 依赖 |
| discount | 快速 | 功能一般 |

**推荐**：先实现基础 Markdown（标题/列表/代码/链接），后续可扩展

### 7.3 Diff 视图

**推荐**：自定义 DiffViewComponent，基于 diff_match_patch 或 dtl 库

---

## 八、实现路线图

### Phase 1: 基础框架 (1-2 周)
- [ ] TimelineModel 数据模型
- [ ] TimelineDelegate 基础结构
- [ ] UserMessageDelegate / AIMessageDelegate
- [ ] 基础输入框

### Phase 2: 核心组件 (2-3 周)
- [ ] CodeBlockDelegate + 语法高亮
- [ ] ToolCallDelegate
- [ ] 复制/应用按钮
- [ ] 流式输出支持

### Phase 3: 高级特性 (2-3 周)
- [ ] ThinkingDelegate
- [ ] FileChangeDelegate + Diff 视图
- [ ] 检查点机制
- [ ] 上下文引用 (@)

### Phase 4: 优化完善 (1-2 周)
- [ ] Markdown 渲染
- [ ] 键盘快捷键
- [ ] 主题适配
- [ ] 性能优化

---

## 九、参考资源

### 开源项目
- [assistant-ui](https://github.com/Yonom/assistant-ui) - React AI 聊天组件库
- [Vercel AI SDK](https://vercel.com/blog/ai-sdk-5) - 消息类型设计
- [Cline](https://github.com/cline/cline) - VSCode Agent 插件
- [Continue](https://github.com/continuedev/continue) - 开源 AI 助手

### 设计规范
- [Ant Design X](https://x.ant.design) - AI 聊天组件设计
- [ChatUI](https://chatui.io) - 阿里巴巴聊天 UI

---

*文档创建：2025-03-01*
*基于 20+ 款 AI 编程助手产品分析*
