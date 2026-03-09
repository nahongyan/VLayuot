/**
 * @file chat_delegate.cpp
 * @brief 聊天消息委托实现
 */

#include "chat_delegate.h"

ChatDelegate::ChatDelegate(QObject* parent)
    : VLayout::DelegateController(parent)
{
    using namespace VLayout;

    // 添加头像组件
    addItem<AvatarComponent>("avatar", 36);

    // 添加消息文本组件
    addItem<LabelComponent>("message", -1);

    // 添加时间戳组件
    addItem<LabelComponent>("time", -1);

    // 设置布局：水平排列 [头像] [消息区域]
    setLayout(HBox(12, 12, 12, 12, 12, {
        Item("avatar", {36, 36}),
        VBox(0, 4, 0, 0, 4, {
            Stretch("message"),
            Item("time", {-1, 20}),
        }),
    }));

    // 绑定数据
    bindTo("message").text(MessageTextRole);
    bindTo("time").text(MessageTimeRole);

    // 根据消息类型设置头像颜色
    bindTo("avatar").property("backgroundColor", MessageTypeRole, [](const QVariant& value) {
        return value.toInt() == 0 ? "#0078d4" : "#107c10";  // 用户: 蓝色, AI: 绿色
    });

    // 设置固定大小提示
    setFixedSizeHint({400, 60});
}
