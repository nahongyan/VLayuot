#ifndef OLLAMA_ADAPTER_H
#define OLLAMA_ADAPTER_H

/**
 * @file ollama_adapter.h
 * @brief Ollama API 适配器
 *
 * 直接与 Ollama HTTP API 通信。
 */

#include "adapter.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace AISDK {

/**
 * @class OllamaAdapter
 * @brief Ollama 适配器
 *
 * 通过 HTTP API 与本地 Ollama 服务通信，支持：
 * - 流式响应
 * - 多种模型
 * - 简单易用
 */
class OllamaAdapter : public CLIAdapter
{
    Q_OBJECT

public:
    explicit OllamaAdapter(QObject *parent = nullptr);
    ~OllamaAdapter() override;

    // ========== CLIAdapter 接口 ==========
    QString name() const override { return QStringLiteral("Ollama"); }
    QString version() const override;

    bool initialize(const AdapterConfig &config) override;
    bool isReady() const override;
    void shutdown() override;

    bool supportsTools() const override { return false; }
    bool supportsStreaming() const override { return true; }
    bool supportsImages() const override { return true; }
    bool supportsMCP() const override { return false; }

    void chat(const QString &message,
              const QJsonArray &history = {},
              const QJsonObject &options = {}) override;
    void cancel() override;

    // ========== Ollama 特有功能 ==========
    /// 设置 API URL (默认 http://localhost:11434)
    void setApiUrl(const QString &url);

    /// 获取可用模型列表
    QStringList availableModels() const;

    /// 检查服务状态
    void checkStatus();

signals:
    /// 服务状态改变
    void statusChanged(bool connected);

private slots:
    void onChatReadyRead();
    void onChatFinished();

private:
    void processStreamLine(const QString &line);

    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_statusTimer = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QString m_apiUrl;
    QString m_currentContent;
    QString m_currentThinking;
    bool m_ready = false;
};

} // namespace AISDK

#endif // OLLAMA_ADAPTER_H
