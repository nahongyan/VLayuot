#ifndef ADAPTER_H
#define ADAPTER_H

/**
 * @file adapter.h
 * @brief CLI 适配器基类
 *
 * 定义与成熟 CLI 工具（如 Crush、OpenCode、Aider）通信的统一接口。
 */

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QDebug>
#include <functional>

#include "../global.h"

namespace AISDK {

/**
 * @class AdapterConfig
 * @brief 适配器配置
 */
struct AdapterConfig
{
    QString executable;       // CLI 可执行文件路径
    QString model;            // 模型名称
    QString provider;         // 提供商 (ollama, openai, anthropic 等)
    QString apiUrl;           // API URL (可选)
    QStringList extraArgs;    // 额外参数
    int timeoutMs = 60000;    // 超时时间（毫秒）
};

/**
 * @class MessageChunk
 * @brief 消息块
 */
struct MessageChunk
{
    enum Type {
        Text,           // 文本内容
        Thinking,       // 思考内容
        ToolCall,       // 工具调用
        ToolResult,     // 工具结果
        Error,          // 错误
        Done            // 完成
    };

    Type type;
    QString content;
    QString id;         // 用于 tool call/result
    QString name;       // 用于 tool call
    QJsonObject input;  // 工具输入参数
    bool isError = false;
};

/**
 * @class CLIAdapter
 * @brief CLI 适配器基类
 *
 * 提供与外部 CLI 工具通信的统一接口。
 */
class AISDK_EXPORT CLIAdapter : public QObject
{
    Q_OBJECT

public:
    explicit CLIAdapter(QObject *parent = nullptr);
    virtual ~CLIAdapter() = default;

    // ========== 适配器信息 ==========
    virtual QString name() const = 0;
    virtual QString version() const = 0;

    // ========== 生命周期 ==========
    virtual bool initialize(const AdapterConfig &config) = 0;
    virtual bool isReady() const = 0;
    virtual void shutdown() = 0;

    // ========== 功能查询 ==========
    virtual bool supportsTools() const { return false; }
    virtual bool supportsStreaming() const { return false; }
    virtual bool supportsImages() const { return false; }
    virtual bool supportsMCP() const { return false; }

    // ========== 对话 ==========
    /// 发送消息并获取流式响应
    virtual void chat(const QString &message,
                      const QJsonArray &history = {},
                      const QJsonObject &options = {}) = 0;

    /// 取消当前请求
    virtual void cancel() = 0;

    // ========== 工具 ==========
    /// 注册工具（如果 CLI 支持）
    virtual void registerTool(const QString &name, const QJsonObject &schema) {
        Q_UNUSED(name); Q_UNUSED(schema);
    }

    /// 获取可用工具列表
    virtual QJsonArray availableTools() const { return {}; }

signals:
    /// 收到消息块
    void chunkReceived(const MessageChunk &chunk);

    /// 收到部分文本
    void partialTextReceived(const QString &text);

    /// 收到完整响应
    void fullResponseReceived(const QString &text);

    /// 思考内容
    void thinkingReceived(const QString &thinking);

    /// 工具调用
    void toolCallReceived(const QString &id, const QString &name, const QJsonObject &input);

    /// 工具结果
    void toolResultReceived(const QString &id, const QString &result, bool isError);

    /// 错误
    void errorOccurred(const QString &error);

    /// 就绪状态改变
    void readyChanged(bool ready);

protected:
    AdapterConfig m_config;

    /// 设置错误（辅助方法）
    void setError(const QString &error) {
        qWarning() << "[CLIAdapter] Error:" << error;
        emit errorOccurred(error);
    }
};

} // namespace AISDK

#endif // ADAPTER_H
