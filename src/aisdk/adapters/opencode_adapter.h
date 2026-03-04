#ifndef OPENCODE_ADAPTER_H
#define OPENCODE_ADAPTER_H

/**
 * @file opencode_adapter.h
 * @brief OpenCode CLI 适配器
 *
 * 通过 HTTP API 与 OpenCode 服务器通信。
 * https://github.com/sst/opencode
 */

#include "adapter.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace AISDK {

/**
 * @class OpenCodeAdapter
 * @brief OpenCode 适配器
 *
 * 通过 HTTP API 与 OpenCode 服务器通信，支持：
 * - 流式响应
 * - 工具调用
 * - 多种模型（Ollama, OpenAI, Anthropic 等）
 */
class OpenCodeAdapter : public CLIAdapter
{
    Q_OBJECT

public:
    explicit OpenCodeAdapter(QObject *parent = nullptr);
    ~OpenCodeAdapter() override;

    // ========== CLIAdapter 接口 ==========
    QString name() const override { return QStringLiteral("OpenCode"); }
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

    // ========== OpenCode 特有功能 ==========
    /// 设置服务器 URL
    void setServerUrl(const QString &url);

    /// 检查服务器状态
    void checkServerStatus();

signals:
    /// 服务器状态改变
    void serverStatusChanged(bool connected);

private slots:
    void onChatReplyFinished();
    void onChatReadyRead();

private:
    void processStreamLine(const QString &line);
    void sendHttpRequest(const QJsonObject &payload);

    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_statusTimer = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QString m_serverUrl;
    QString m_currentContent;
    bool m_ready = false;
};

} // namespace AISDK

#endif // OPENCODE_ADAPTER_H
