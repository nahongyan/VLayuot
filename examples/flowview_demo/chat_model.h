#ifndef CHAT_MODEL_H
#define CHAT_MODEL_H

/**
 * @file chat_model.h
 * @brief 聊天消息数据模型
 */

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

/**
 * @brief 消息类型
 */
enum class MessageType {
    User,       ///< 用户消息
    Assistant   ///< AI/助手消息
};

/**
 * @brief 聊天消息结构
 */
struct ChatMessage
{
    QString text;           ///< 消息内容
    MessageType type;       ///< 消息类型
    QDateTime timestamp;    ///< 时间戳
};

/**
 * @class ChatModel
 * @brief 聊天消息数据模型
 *
 * 为 FlowView 提供数据。支持大量消息的高效管理。
 */
class ChatModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        MessageTextRole = Qt::UserRole + 1,
        MessageTypeRole,
        MessageTimeRole,
        AvatarUrlRole
    };

    explicit ChatModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 数据操作
    void addMessage(const QString& text, MessageType type);
    void populate(int count);
    void clear();

private:
    QVector<ChatMessage> m_messages;
};

#endif // CHAT_MODEL_H
