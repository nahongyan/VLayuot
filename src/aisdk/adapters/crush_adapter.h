#ifndef CRUSH_ADAPTER_H
#define CRUSH_ADAPTER_H

/**
 * @file crush_adapter.h
 * @brief Crush CLI 适配器
 *
 * 与 Charmbracelet Crush CLI 通信的适配器实现。
 * https://github.com/charmbracelet/crush
 */

#include "adapter.h"

#include <QProcess>
#include <QTimer>
#include <QQueue>

namespace AISDK {

/**
 * @class CrushAdapter
 * @brief Crush CLI 适配器
 *
 * 通过 QProcess 与 Crush CLI 通信，支持：
 * - 流式响应
 * - 工具调用
 * - MCP 协议
 * - 多种模型（Ollama, OpenAI, Anthropic 等）
 */
class CrushAdapter : public CLIAdapter
{
    Q_OBJECT

public:
    explicit CrushAdapter(QObject *parent = nullptr);
    ~CrushAdapter() override;

    // ========== CLIAdapter 接口 ==========
    QString name() const override { return QStringLiteral("Crush"); }
    QString version() const override;

    bool initialize(const AdapterConfig &config) override;
    bool isReady() const override;
    void shutdown() override;

    bool supportsTools() const override { return true; }
    bool supportsStreaming() const override { return true; }
    bool supportsImages() const override { return true; }
    bool supportsMCP() const override { return true; }

    void chat(const QString &message,
              const QJsonArray &history = {},
              const QJsonObject &options = {}) override;
    void cancel() override;

    // ========== Crush 特有功能 ==========
    /// 加载 MCP 配置文件
    bool loadMCPConfig(const QString &configPath);

    /// 设置系统提示
    void setSystemPrompt(const QString &prompt);

    /// 获取可用模型列表
    QStringList availableModels() const;

signals:
    /// 进程状态改变
    void processStateChanged(QProcess::ProcessState state);

private slots:
    void onProcessReadyRead();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTimeout();

private:
    void startProcess();
    void stopProcess();
    void writeToProcess(const QJsonObject &message);
    void processLine(const QString &line);
    void handleResponse(const QJsonObject &response);
    void setError(const QString &error);

    QProcess *m_process = nullptr;
    QTimer *m_timeoutTimer = nullptr;
    QString m_buffer;
    bool m_ready = false;
    QString m_systemPrompt;
    QString m_currentRequestId;
    QJsonObject m_mcpConfig;
};

} // namespace AISDK

#endif // CRUSH_ADAPTER_H
