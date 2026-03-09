#include "ollama_adapter.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

namespace AISDK {

OllamaAdapter::OllamaAdapter(QObject *parent)
    : CLIAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_statusTimer(new QTimer(this))
    , m_apiUrl(QStringLiteral("http://localhost:11434"))
{
    // 定期检查服务状态
    connect(m_statusTimer, &QTimer::timeout, this, &OllamaAdapter::checkStatus);
}

OllamaAdapter::~OllamaAdapter()
{
    shutdown();
}

QString OllamaAdapter::version() const
{
    return QStringLiteral("1.0");
}

bool OllamaAdapter::initialize(const AdapterConfig &config)
{
    m_config = config;

    // 设置 API URL
    if (!config.apiUrl.isEmpty()) {
        m_apiUrl = config.apiUrl;
    }

    qDebug() << "[OllamaAdapter] Initializing with API:" << m_apiUrl
             << "model:" << config.model;

    // 检查服务状态
    checkStatus();

    // 启动状态检查定时器
    m_statusTimer->start(30000);  // 每 30 秒检查一次

    return true;
}

bool OllamaAdapter::isReady() const
{
    return m_ready;
}

void OllamaAdapter::shutdown()
{
    qDebug() << "[OllamaAdapter] Shutting down";

    m_statusTimer->stop();

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_ready = false;
    emit readyChanged(false);
}

void OllamaAdapter::setApiUrl(const QString &url)
{
    m_apiUrl = url;
    checkStatus();
}

QStringList OllamaAdapter::availableModels() const
{
    // 这个应该异步获取，这里返回空列表
    return {};
}

void OllamaAdapter::checkStatus()
{
    QNetworkRequest request(QUrl(m_apiUrl + QStringLiteral("/api/tags")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        bool wasReady = m_ready;

        if (reply->error() == QNetworkReply::NoError) {
            m_ready = true;
            qDebug() << "[OllamaAdapter] Service is ready";
        } else {
            m_ready = false;
            qDebug() << "[OllamaAdapter] Service not available:" << reply->errorString();
        }

        if (wasReady != m_ready) {
            emit readyChanged(m_ready);
            emit statusChanged(m_ready);
        }

        reply->deleteLater();
    });
}

void OllamaAdapter::chat(const QString &message,
                          const QJsonArray &history,
                          const QJsonObject &options)
{
    if (!m_ready) {
        setError(tr("Ollama service is not ready"));
        return;
    }

    // 取消之前的请求
    cancel();

    qDebug() << "[OllamaAdapter] Sending chat message:" << message.left(100);

    m_currentContent.clear();
    m_currentThinking.clear();

    // 构建消息数组
    QJsonArray messages;

    // 添加历史消息
    for (const QJsonValue &val : history) {
        messages.append(val);
    }

    // 添加当前消息
    QJsonObject userMessage;
    userMessage[QStringLiteral("role")] = QStringLiteral("user");
    userMessage[QStringLiteral("content")] = message;
    messages.append(userMessage);

    // 构建请求
    QJsonObject payload;
    payload[QStringLiteral("model")] = m_config.model.isEmpty()
        ? QStringLiteral("qwen3.5:27b")
        : m_config.model;
    payload[QStringLiteral("messages")] = messages;
    payload[QStringLiteral("stream")] = true;

    // 合并选项
    for (auto it = options.begin(); it != options.end(); ++it) {
        payload[it.key()] = it.value();
    }

    // 发送请求
    QNetworkRequest request(QUrl(m_apiUrl + QStringLiteral("/api/chat")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QByteArray data = QJsonDocument(payload).toJson();
    m_currentReply = m_networkManager->post(request, data);

    connect(m_currentReply, &QNetworkReply::readyRead, this, &OllamaAdapter::onChatReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &OllamaAdapter::onChatFinished);
}

void OllamaAdapter::cancel()
{
    if (m_currentReply) {
        qDebug() << "[OllamaAdapter] Cancelling current request";
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void OllamaAdapter::onChatReadyRead()
{
    if (!m_currentReply) return;

    QByteArray data = m_currentReply->readAll();
    QString text = QString::fromUtf8(data);

    // Ollama 每行是一个 JSON 对象
    QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        processStreamLine(line);
    }
}

void OllamaAdapter::processStreamLine(const QString &line)
{
    if (line.trimmed().isEmpty()) return;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

    if (doc.isNull()) {
        qDebug() << "[OllamaAdapter] JSON parse error:" << error.errorString();
        return;
    }

    QJsonObject obj = doc.object();

    // 检查错误
    if (obj.contains(QStringLiteral("error"))) {
        QString errorMsg = obj[QStringLiteral("error")].toString();
        setError(errorMsg);
        return;
    }

    // 获取消息内容
    QJsonObject message = obj[QStringLiteral("message")].toObject();

    // 处理 thinking 字段（思考过程）
    QString thinking = message[QStringLiteral("thinking")].toString();
    if (!thinking.isEmpty()) {
        m_currentThinking += thinking;
        // 发送累积的思考内容
        emit thinkingReceived(m_currentThinking);
    }

    // 处理 content 字段（实际回复）
    QString content = message[QStringLiteral("content")].toString();
    if (!content.isEmpty()) {
        m_currentContent += content;
        // 发送累积的完整内容，而不是增量
        emit partialTextReceived(m_currentContent);
    }

    // 检查是否完成
    if (obj[QStringLiteral("done")].toBool()) {
        emit fullResponseReceived(m_currentContent);
    }
}

void OllamaAdapter::onChatFinished()
{
    if (!m_currentReply) return;

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError &&
        reply->error() != QNetworkReply::OperationCanceledError) {
        QString errorMsg = reply->errorString();
        qDebug() << "[OllamaAdapter] Request error:" << errorMsg;
        setError(errorMsg);
    }

    reply->deleteLater();
}

} // namespace AISDK
