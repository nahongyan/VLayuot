#include "opencode_adapter.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

namespace AISDK {

OpenCodeAdapter::OpenCodeAdapter(QObject *parent)
    : CLIAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_statusTimer(new QTimer(this))
    , m_serverUrl(QStringLiteral("http://localhost:8080"))
{
    // 定期检查服务器状态
    connect(m_statusTimer, &QTimer::timeout, this, &OpenCodeAdapter::checkServerStatus);
}

OpenCodeAdapter::~OpenCodeAdapter()
{
    shutdown();
}

QString OpenCodeAdapter::version() const
{
    return QStringLiteral("1.0");
}

bool OpenCodeAdapter::initialize(const AdapterConfig &config)
{
    m_config = config;

    // 设置服务器 URL
    if (!config.apiUrl.isEmpty()) {
        m_serverUrl = config.apiUrl;
    }

    qDebug() << "[OpenCodeAdapter] Initializing with server:" << m_serverUrl;

    // 检查服务器状态
    checkServerStatus();

    // 启动状态检查定时器
    m_statusTimer->start(30000);  // 每 30 秒检查一次

    return true;
}

bool OpenCodeAdapter::isReady() const
{
    return m_ready;
}

void OpenCodeAdapter::shutdown()
{
    qDebug() << "[OpenCodeAdapter] Shutting down";

    m_statusTimer->stop();

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_ready = false;
    emit readyChanged(false);
}

void OpenCodeAdapter::setServerUrl(const QString &url)
{
    m_serverUrl = url;
    checkServerStatus();
}

void OpenCodeAdapter::checkServerStatus()
{
    QNetworkRequest request(QUrl(m_serverUrl + QStringLiteral("/api/status")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        bool wasReady = m_ready;

        if (reply->error() == QNetworkReply::NoError) {
            m_ready = true;
            qDebug() << "[OpenCodeAdapter] Server is ready";
        } else {
            m_ready = false;
            qDebug() << "[OpenCodeAdapter] Server not available:" << reply->errorString();
        }

        if (wasReady != m_ready) {
            emit readyChanged(m_ready);
            emit serverStatusChanged(m_ready);
        }

        reply->deleteLater();
    });
}

void OpenCodeAdapter::chat(const QString &message,
                            const QJsonArray &history,
                            const QJsonObject &options)
{
    if (!m_ready) {
        setError(tr("OpenCode server is not ready"));
        return;
    }

    // 取消之前的请求
    cancel();

    qDebug() << "[OpenCodeAdapter] Sending chat message:" << message.left(100);

    m_currentContent.clear();

    // 构建请求
    QJsonObject payload;
    payload[QStringLiteral("message")] = message;

    if (!m_config.model.isEmpty()) {
        payload[QStringLiteral("model")] = m_config.model;
    }

    if (!history.isEmpty()) {
        payload[QStringLiteral("history")] = history;
    }

    // 合并选项
    for (auto it = options.begin(); it != options.end(); ++it) {
        payload[it.key()] = it.value();
    }

    // 添加流式请求标志
    payload[QStringLiteral("stream")] = true;

    sendHttpRequest(payload);
}

void OpenCodeAdapter::cancel()
{
    if (m_currentReply) {
        qDebug() << "[OpenCodeAdapter] Cancelling current request";
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void OpenCodeAdapter::sendHttpRequest(const QJsonObject &payload)
{
    QNetworkRequest request(QUrl(m_serverUrl + QStringLiteral("/api/chat")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QByteArray data = QJsonDocument(payload).toJson();
    m_currentReply = m_networkManager->post(request, data);

    connect(m_currentReply, &QNetworkReply::readyRead, this, &OpenCodeAdapter::onChatReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &OpenCodeAdapter::onChatReplyFinished);
}

void OpenCodeAdapter::onChatReadyRead()
{
    if (!m_currentReply) return;

    QByteArray data = m_currentReply->readAll();
    QString text = QString::fromUtf8(data);

    // 处理 SSE 格式
    QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        processStreamLine(line);
    }
}

void OpenCodeAdapter::processStreamLine(const QString &line)
{
    // 处理 SSE 格式: data: {...}
    if (line.startsWith(QStringLiteral("data: "))) {
        QString jsonStr = line.mid(6);

        if (jsonStr == QStringLiteral("[DONE]")) {
            // 流结束
            emit fullResponseReceived(m_currentContent);
            return;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

        if (doc.isNull()) {
            qDebug() << "[OpenCodeAdapter] JSON parse error:" << error.errorString();
            return;
        }

        QJsonObject obj = doc.object();
        QString type = obj[QStringLiteral("type")].toString();

        if (type == QStringLiteral("content") || type == QStringLiteral("text")) {
            QString content = obj[QStringLiteral("content")].toString();
            if (content.isEmpty()) {
                content = obj[QStringLiteral("text")].toString();
            }
            if (content.isEmpty()) {
                content = obj[QStringLiteral("delta")].toString();
            }

            if (!content.isEmpty()) {
                m_currentContent += content;
                emit partialTextReceived(content);
            }
        }
        else if (type == QStringLiteral("thinking")) {
            QString thinking = obj[QStringLiteral("content")].toString();
            emit thinkingReceived(thinking);
        }
        else if (type == QStringLiteral("tool_call")) {
            QString id = obj[QStringLiteral("id")].toString();
            QString name = obj[QStringLiteral("name")].toString();
            QJsonObject input = obj[QStringLiteral("input")].toObject();
            emit toolCallReceived(id, name, input);
        }
        else if (type == QStringLiteral("tool_result")) {
            QString id = obj[QStringLiteral("id")].toString();
            QString result = obj[QStringLiteral("result")].toString();
            bool isError = obj[QStringLiteral("error")].toBool();
            emit toolResultReceived(id, result, isError);
        }
        else if (type == QStringLiteral("error")) {
            QString errorMsg = obj[QStringLiteral("message")].toString();
            if (errorMsg.isEmpty()) {
                errorMsg = obj[QStringLiteral("error")].toString();
            }
            setError(errorMsg);
        }
    }
    // 处理普通 JSON 格式
    else if (line.startsWith(QLatin1Char('{'))) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

        if (!doc.isNull()) {
            QJsonObject obj = doc.object();

            // 检查是否有 content 字段
            QString content = obj[QStringLiteral("content")].toString();
            if (!content.isEmpty()) {
                m_currentContent += content;
                emit partialTextReceived(content);
            }

            // 检查是否有 done 字段
            if (obj[QStringLiteral("done")].toBool()) {
                emit fullResponseReceived(m_currentContent);
            }
        }
    }
}

void OpenCodeAdapter::onChatReplyFinished()
{
    if (!m_currentReply) return;

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError) {
        QString errorMsg = reply->errorString();
        qDebug() << "[OpenCodeAdapter] Request error:" << errorMsg;
        setError(errorMsg);
    }

    reply->deleteLater();
}

} // namespace AISDK
