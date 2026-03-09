#ifndef CHAT_DELEGATE_H
#define CHAT_DELEGATE_H

/**
 * @file chat_delegate.h
 * @brief 聊天消息委托定义
 *
 * 使用 VLayout 的 DelegateController 实现聊天消息的渲染。
 */

#include <vlayout/framework.h>

/**
 * @brief 消息角色枚举
 */
enum ChatRoles {
    MessageTextRole = Qt::UserRole + 1,  ///< 消息文本
    MessageTypeRole,                      ///< 消息类型 (User/Assistant)
    MessageTimeRole,                      ///< 消息时间
    AvatarUrlRole                         ///< 头像URL（可选）
};

/**
 * @class ChatDelegate
 * @brief 聊天消息委托
 *
 * 使用 VLayout 的声明式 API 定义聊天消息的布局和绑定。
 */
class ChatDelegate : public VLayout::DelegateController
{
public:
    explicit ChatDelegate(QObject* parent = nullptr);
};

#endif // CHAT_DELEGATE_H
