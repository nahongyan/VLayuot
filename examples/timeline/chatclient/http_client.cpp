#include "http_client.h"

#include <QJsonDocument>
#include <QMutexLocker>
#include <QDebug>

namespace ChatClient {

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{}

HttpClient::~HttpClient()
{
    QMutexLocker locker(&m_mutex);
    for (auto *reply : std::as_const(m_activeRequests)) {
        reply->abort();
        reply->deleteLater();
    }
    m_activeRequests.clear();
}

void HttpClient::postStreaming(const QString &requestId, const QNetworkRequest &request,
                                const QJsonObject &payload)
{
    QJsonDocument doc(payload);
    qDebug() << "HttpClient: POST streaming" << request.url().toString();

    QNetworkReply *reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));
    addActiveRequest(reply, requestId);

    connect(reply, &QNetworkReply::readyRead, this, &HttpClient::onReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onStreamingFinished);
}

void HttpClient::cancelRequest(const QString &requestId)
{
    QMutexLocker locker(&m_mutex);
    auto it = m_activeRequests.find(requestId);
    if (it != m_activeRequests.end()) {
        QNetworkReply *reply = it.value();
        if (reply) {
            reply->disconnect();
            reply->abort();
            reply->deleteLater();
        }
        m_activeRequests.erase(it);
        qDebug() << "HttpClient: Cancelled request:" << requestId;
    }
}

bool HttpClient::hasActiveRequest(const QString &requestId) const
{
    QMutexLocker locker(&m_mutex);
    return m_activeRequests.contains(requestId);
}

void HttpClient::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply->isFinished())
        return;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 400)
        return;

    QString requestId = findRequestId(reply);
    if (requestId.isEmpty())
        return;

    QByteArray data = reply->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(requestId, data);
    }
}

void HttpClient::onStreamingFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseBody = reply->readAll();
    QNetworkReply::NetworkError networkError = reply->error();
    QString networkErrorString = reply->errorString();

    reply->disconnect();

    QString requestId;
    std::optional<QString> error;

    {
        QMutexLocker locker(&m_mutex);
        for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
            if (it.value() == reply) {
                requestId = it.key();
                m_activeRequests.erase(it);
                break;
            }
        }

        if (requestId.isEmpty()) {
            reply->deleteLater();
            return;
        }

        bool hasError = (networkError != QNetworkReply::NoError) || (statusCode >= 400);
        if (hasError) {
            error = parseErrorFromResponse(statusCode, responseBody, networkErrorString);
        }

        qDebug() << "HttpClient: Request" << requestId << "finished, status:" << statusCode;
        if (error) {
            qDebug() << "HttpClient: Error:" << *error;
        }
    }

    reply->deleteLater();

    if (!requestId.isEmpty()) {
        emit requestFinished(requestId, error);
    }
}

QString HttpClient::findRequestId(QNetworkReply *reply)
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
        if (it.value() == reply)
            return it.key();
    }
    return QString();
}

void HttpClient::addActiveRequest(QNetworkReply *reply, const QString &requestId)
{
    QMutexLocker locker(&m_mutex);
    m_activeRequests[requestId] = reply;
    qDebug() << "HttpClient: Added active request:" << requestId;
}

QString HttpClient::parseErrorFromResponse(int statusCode, const QByteArray &responseBody,
                                            const QString &networkErrorString)
{
    if (!responseBody.isEmpty()) {
        QJsonDocument errorDoc = QJsonDocument::fromJson(responseBody);
        if (!errorDoc.isNull() && errorDoc.isObject()) {
            QJsonObject errorObj = errorDoc.object();
            if (errorObj.contains(QStringLiteral("error"))) {
                auto error = errorObj[QStringLiteral("error")];
                if (error.isString()) {
                    return QString(QStringLiteral("HTTP %1: %2")).arg(statusCode).arg(error.toString());
                } else if (error.isObject()) {
                    QJsonObject errorObjInner = error.toObject();
                    QString message = errorObjInner[QStringLiteral("message")].toString();
                    QString type = errorObjInner[QStringLiteral("type")].toString();
                    QString code = errorObjInner[QStringLiteral("code")].toString();

                    QString errorMsg = QString(QStringLiteral("HTTP %1: %2")).arg(statusCode).arg(message);
                    if (!type.isEmpty())
                        errorMsg += QString(QStringLiteral(" (type: %1)")).arg(type);
                    if (!code.isEmpty())
                        errorMsg += QString(QStringLiteral(" (code: %1)")).arg(code);
                    return errorMsg;
                }
            }
            return QString(QStringLiteral("HTTP %1: %2"))
                .arg(statusCode)
                .arg(QString::fromUtf8(responseBody));
        }
    }
    return QString(QStringLiteral("HTTP %1: %2")).arg(statusCode).arg(networkErrorString);
}

} // namespace ChatClient
