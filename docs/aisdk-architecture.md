# AISDK 架构设计

> 核心原则：**不重复造轮子，对接成熟 CLI**

## 推荐方案：Qt + Crush (Charmbracelet)

### 首选：[Crush](https://github.com/charmbracelet/crush)

**Charmbracelet 出品** - 终端 UI 大师（Bubbletea、Lipgloss、Glamour）

| 特性 | 说明 |
|------|------|
| **语言** | Go（单二进制，跨平台） |
| **模型支持** | OpenAI, Anthropic, Gemini, Groq, Bedrock, Vertex AI, **Ollama**, LM Studio |
| **MCP 协议** | ✅ stdio, http, sse |
| **LSP 集成** | ✅ 自动获取代码上下文 |
| **Skills 系统** | ✅ 可复用技能包 |
| **本地模型** | ✅ Ollama, LM Studio |
| **许可证** | FSL-1.1-MIT |

```bash
# 安装
brew install charmbracelet/tap/crush  # macOS
winget install charmbracelet.crush    # Windows
```

### 其他可选 CLI

| CLI | Stars | 语言 | 特点 |
|-----|-------|------|------|
| [OpenCode](https://github.com/sst/opencode) | 95k+ | Go/TS | 客户端/服务器架构 |
| [Claude Code Open](https://github.com/kill136/claude-code-open) | - | TS | 37+ 工具，Multi-Agent |
| [Aider](https://github.com/aider-ai/aider) | 30k+ | Python | Git 深度集成 |

### 集成架构

```text
┌─────────────────────────────────────────────────────────────────┐
│                      Qt Application                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │  Timeline   │  │   Chat UI   │  │  Code Editor│             │
│  └─────────────┘  └─────────────┘  └─────────────┘             │
│                           │                                      │
│                           ▼                                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                    AISDK Adapter Layer                   │   │
│  │  ┌───────────────┐  ┌───────────────┐  ┌─────────────┐  │   │
│  │  │ CrushAdapter  │  │ OpenCodeAdapter│  │AiderAdapter │  │   │
│  │  │    (首选)      │  │               │  │             │  │   │
│  │  └───────────────┘  └───────────────┘  └─────────────┘  │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│     Crush     │   │   OpenCode    │   │    Aider      │
│   (子进程)     │   │   (子进程)     │   │   (子进程)     │
│               │   │               │   │               │
│ - Ollama 支持 │   │ - 75+ 模型    │   │ - Git 集成    │
│ - MCP 协议    │   │ - MCP 协议    │   │ - 多模型      │
│ - Skills 系统 │   │ - LSP 支持    │   │ - 上下文管理  │
│ - Charmbracelet│   │ - 服务器模式  │   │               │
└───────────────┘   └───────────────┘   └───────────────┘
```

## 集成方式

### 方式 1: QProcess + stdio (推荐)

```cpp
class OpenCodeAdapter : public QObject {
    Q_OBJECT
public:
    void start() {
        m_process = new QProcess(this);
        m_process->start("opencode", {"--json-mode"});

        connect(m_process, &QProcess::readyReadStandardOutput, [=]() {
            QByteArray data = m_process->readAllStandardOutput();
            handleResponse(data);
        });
    }

    void sendMessage(const QString &message) {
        QJsonObject req;
        req["type"] = "chat";
        req["content"] = message;
        m_process->write(QJsonDocument(req).toJson() + "\n");
    }

private:
    QProcess *m_process;
};
```

### 方式 2: HTTP API

```cpp
// OpenCode 支持客户端/服务器模式
// 启动服务器: opencode server --port 8080

class OpenCodeClient : public QObject {
    void chat(const QString &message) {
        QNetworkRequest request(QUrl("http://localhost:8080/api/chat"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject body;
        body["message"] = message;
        body["model"] = "qwen3.5:27b";

        m_http->post(request, QJsonDocument(body).toJson());
    }
};
```

### 方式 3: MCP 协议 (最灵活)

```cpp
// 通过 MCP 协议与 CLI 通信
class MCPClient {
    void connectToCLI(const QString &command, const QStringList &args) {
        m_process->start(command, args);
        // 发送 MCP initialize
        sendMCPRequest("initialize", {...});
    }

    void callTool(const QString &name, const QJsonObject &args) {
        sendMCPRequest("tools/call", {
            {"name", name},
            {"arguments", args}
        });
    }
};
```

## 当前实现状态

### ✅ 已完成

```text
src/aisdk/
├── adapters/                    # 适配器层 (已实现)
│   ├── adapter.h               # 适配器基类 ✅
│   └── crush_adapter.*         # Crush 适配器 ✅
│
├── core/                       # 核心模块 (已实现)
│   ├── ai_types.h              # 类型定义 ✅
│   ├── http_client.*           # HTTP 客户端 ✅
│   ├── sse_buffer.*            # SSE 缓冲区 ✅
│   ├── content_blocks.h        # 内容块 ✅
│   └── provider.*              # Provider 基类 ✅
│
├── providers/                  # Provider 实现 (已实现)
│   ├── ollama_provider.*       # Ollama Provider ✅
│   └── ollama_message.*        # Ollama 消息处理 ✅
│
├── tools/                      # 工具系统 (已实现)
│   ├── base_tool.h             # 工具基类 ✅
│   ├── echo_tool.h             # 示例工具 ✅
│   └── tools_manager.*         # 工具管理器 ✅
│
├── mcp/                        # MCP 协议 (已实现)
│   ├── mcp_client.*            # MCP 客户端 ✅
│   ├── mcp_tool.*              # MCP 工具包装 ✅
│   └── mcp_manager.*           # MCP 管理器 ✅
│
├── settings/                   # 配置 (已实现)
│   └── ai_settings.*           # AI 设置 ✅
│
├── ai_client.h/cpp             # 统一入口 ✅
└── aisdk.pri                   # qmake 配置 ✅
```

## 使用方法

### 方法 1: 直接使用 Ollama Provider

```cpp
#include <aisdk/ai_client.h>

// 初始化
AISDK::AIClient *client = new AISDK::AIClient(this);
client->initialize("Ollama");

// 发送消息
client->sendMessageAsync("你好，请介绍一下你自己", "", [](const QString &response) {
    qDebug() << "Response:" << response;
});
```

### 方法 2: 使用 Crush 适配器（推荐）

```cpp
#include <aisdk/adapters/crush_adapter.h>

// 创建适配器
AISDK::CrushAdapter *adapter = new AISDK::CrushAdapter(this);

// 配置
AISDK::AdapterConfig config;
config.executable = "crush";
config.model = "qwen3.5:27b";
config.provider = "ollama";

// 初始化
adapter->initialize(config);

// 连接信号
connect(adapter, &CrushAdapter::partialTextReceived, this, [](const QString &text) {
    qDebug() << "Chunk:" << text;
});

connect(adapter, &CrushAdapter::fullResponseReceived, this, [](const QString &text) {
    qDebug() << "Full:" << text;
});

// 发送消息
adapter->chat("你好");
```

## 安装成熟 CLI

```bash
# Crush (推荐)
brew install charmbracelet/tap/crush  # macOS
winget install charmbracelet.crush    # Windows

# 或从 GitHub 下载
# https://github.com/charmbracelet/crush/releases
```

## 配置文件示例

`examples/timeline/crush_config.json`:

```json
{
  "model": "ollama:qwen3.5:27b",
  "provider": "ollama",
  "mcpServers": {
    "filesystem": {
      "command": "mcp-server-filesystem",
      "args": ["--root", "."]
    }
  },
  "options": {
    "temperature": 0.7,
    "maxTokens": 4096
  }
}
```

## 实现步骤

### Phase 1: ✅ 基础架构

1. ✅ 创建 `Adapter` 基类
2. ✅ 实现 `CrushAdapter`
3. ✅ 配置 qmake 包含

### Phase 2: 集成测试

1. 安装 Crush CLI
2. 测试基本通信
3. 测试 Ollama + qwen3.5:27b

### Phase 3: Timeline 集成

1. 在 Timeline Widget 中使用适配器
2. 显示 AI 响应
3. 支持工具调用结果

## 为什么选择对接而不是自己实现？

| 对比项 | 自己实现 | 对接成熟 CLI |
|--------|---------|-------------|
| 开发时间 | 数月 | 数天 |
| 功能完整度 | 有限 | 完整 (37+ 工具) |
| 社区支持 | 无 | 活跃 |
| Bug 修复 | 自己负责 | 社区维护 |
| 新模型支持 | 自己适配 | 自动获得 |
| MCP 协议 | 需要实现 | 已支持 |

## 推荐优先级

1. **Crush** - Charmbracelet 出品，Go 单二进制，跨平台，已实现适配器
2. **OpenCode** - 客户端/服务器架构，最容易集成
3. **Claude Code Open** - 功能最完整，MIT 许可
4. **Aider** - 如果需要深度 Git 集成
