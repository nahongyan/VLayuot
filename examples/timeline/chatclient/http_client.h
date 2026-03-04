#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

/**
 * @file http_client.h
 * @brief HTTP 客户端，支持流式响应
 *
 * 基于 QodeAssist 项目的 HttpClient 简化实现。
 */

#include <QHash>
#include <QJsonObject>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <optional>

namespace ChatClient {

/**
 * @class HttpClient
 * @brief HTTP 客户端，支持流式和非流式请求
 */
class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    /// 发送流式 POST 请求
    void postStreaming(const QString &requestId, const QNetworkRequest &request,
                       const QJsonObject &payload);

    /// 取消请求
    void cancelRequest(const QString &requestId);

    /// 检查是否有活跃请求
    bool hasActiveRequest(const QString &requestId) const;

signals:
    /// 收到数据
    void dataReceived(const QString &requestId, const QByteArray &data);
    /// 请求完成
    void requestFinished(const QString &requestId, std::optional<QString> error);

private slots:
    void onReadyRead();
    void onStreamingFinished();

private:
    QString findRequestId(QNetworkReply *reply);
    void addActiveRequest(QNetworkReply *reply, const QString &requestId);
    QString parseErrorFromResponse(int statusCode, const QByteArray &responseBody,
                                   const QString &networkErrorString);

    QNetworkAccessManager *m_manager;
    QHash<QString, QNetworkReply *> m_activeRequests;
    mutable QMutex m_mutex;
};

} // namespace ChatClient

#endif // HTTP_CLIENT_H
