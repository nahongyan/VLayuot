/**
 * @file chat_model.cpp
 * @brief 聊天消息数据模型实现
 */

#include "chat_model.h"
#include <QRandomGenerator>

ChatModel::ChatModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ChatModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_messages.count();
}

QVariant ChatModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.count())
        return QVariant();

    const auto& msg = m_messages[index.row()];

    switch (role) {
    case MessageTextRole:
        return msg.text;
    case MessageTypeRole:
        return static_cast<int>(msg.type);
    case MessageTimeRole:
        return msg.timestamp.toString("hh:mm");
    case AvatarUrlRole:
        return msg.type == MessageType::User ? "user" : "assistant";
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatModel::roleNames() const
{
    return {
        {MessageTextRole, "messageText"},
        {MessageTypeRole, "messageType"},
        {MessageTimeRole, "messageTime"},
        {AvatarUrlRole, "avatarUrl"}
    };
}

void ChatModel::addMessage(const QString& text, MessageType type)
{
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append({text, type, QDateTime::currentDateTime()});
    endInsertRows();
}

void ChatModel::populate(int count)
{
    beginResetModel();
    m_messages.clear();
    m_messages.reserve(count);

    // 示例消息模板
    QStringList userMessages = {
        tr("你好，请帮我写一个排序算法"),
        tr("能解释一下虚拟化列表的原理吗？"),
        tr("如何优化 Qt 应用的性能？"),
        tr("请帮我设计一个数据结构"),
        tr("什么是 MVC 模式？"),
        tr("如何处理大数据量的显示？"),
        tr("介绍一下 Qt 的模型/视图架构"),
        tr("如何实现流畅的滚动效果？"),
        tr("请解释一下 Delegate 模式"),
        tr("Qt 中如何自定义委托？")
    };

    QStringList assistantMessages = {
        tr("好的，我来帮你实现一个快速排序算法..."),
        tr("虚拟化列表的核心思想是只渲染可见区域的项..."),
        tr("优化 Qt 应用性能可以从以下几个方面入手..."),
        tr("根据你的需求，我建议使用以下数据结构..."),
        tr("MVC 模式是一种软件架构模式，它将应用分为..."),
        tr("处理大数据量显示的关键是使用虚拟化技术..."),
        tr("Qt 的模型/视图架构提供了数据和视图的分离..."),
        tr("实现流畅滚动需要注意以下几点..."),
        tr("Delegate 模式是一种设计模式，用于..."),
        tr("在 Qt 中自定义委托需要继承 QStyledItemDelegate...")
    };

    QRandomGenerator* rng = QRandomGenerator::global();

    for (int i = 0; i < count; ++i) {
        bool isUser = i % 2 == 0;
        const QStringList& messages = isUser ? userMessages : assistantMessages;

        QString text = messages[rng->bounded(messages.size())];

        // 添加一些变化
        if (rng->bounded(10) < 3) {
            text += tr(" [#%1]").arg(i);
        }

        m_messages.append({
            text,
            isUser ? MessageType::User : MessageType::Assistant,
            QDateTime::currentDateTime().addSecs(-count + i)
        });
    }

    endResetModel();
}

void ChatModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
