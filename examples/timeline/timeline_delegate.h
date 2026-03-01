#ifndef TIMELINE_DELEGATE_H
#define TIMELINE_DELEGATE_H

/**
 * @file timeline_delegate.h
 * @brief 时间轴委托类定义
 *
 * 定义了用于 AI 编程助手时间轴的委托类。
 */

#include "vlayout/framework.h"
#include "timeline_roles.h"

#include <QStyledItemDelegate>
#include <QHash>

namespace Timeline {

/**
 * @class TimelineDelegate
 * @brief 时间轴统一委托
 *
 * 根据 NodeType 动态切换布局，绘制不同类型的时间轴节点。
 *
 * ## 支持的节点类型
 * - UserMessage: 用户消息气泡
 * - AIMessage: AI 回复气泡
 * - CodeBlock: 代码块
 * - ToolCall: 工具调用卡片
 * - ThinkingStep: 思考过程
 * - TaskList: 任务列表
 *
 * ## 特性
 * - 流式输出支持
 * - 折叠/展开交互
 * - 复制代码功能
 * - 工具状态动画
 */
class TimelineDelegate : public VLayout::DelegateController
{
    Q_OBJECT

public:
    explicit TimelineDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

signals:
    /**
     * @brief 复制代码请求
     * @param nodeId 节点 ID
     * @param code 代码内容
     */
    void copyCodeRequested(const QString& nodeId, const QString& code);

    /**
     * @brief 折叠状态改变
     * @param nodeId 节点 ID
     * @param expanded 是否展开
     */
    void expandedChanged(const QString& nodeId, bool expanded);

    /**
     * @brief 任务状态改变请求
     * @param nodeId 节点 ID
     * @param taskId 任务 ID
     * @param status 新状态
     */
    void taskStatusChangeRequested(const QString& nodeId,
                                   const QString& taskId,
                                   TaskStatus status);

protected:
    /**
     * @brief 根据节点类型设置布局
     */
    void setupLayoutForType(const QModelIndex& index) const;

private:
    // 布局设置方法
    void setupUserMessageLayout(const QModelIndex& index) const;
    void setupAIMessageLayout(const QModelIndex& index) const;
    void setupCodeBlockLayout(const QModelIndex& index) const;
    void setupToolCallLayout(const QModelIndex& index) const;
    void setupThinkingLayout(const QModelIndex& index) const;
    void setupTaskListLayout(const QModelIndex& index) const;

    // 尺寸计算方法
    int calculateMessageHeight(const QString& content, int width) const;
    int calculateCodeBlockHeight(const QString& code, int width) const;
    int calculateToolCallHeight(const QModelIndex& index, int width) const;
    int calculateThinkingHeight(const QStringList& steps, bool expanded) const;
    int calculateTaskListHeight(const QVariantList& tasks, bool expanded) const;

    // 交互区域检测
    QRect getExpandButtonRect(const QRect& itemRect) const;
    QRect getCopyButtonRect(const QRect& itemRect) const;

    // 时间线绘制
    void paintTimeline(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    // 可变状态（用于缓存）
    mutable QHash<QString, bool> m_copiedStates;  ///< 复制状态缓存
    mutable QString m_lastNodeId;                  ///< 上次处理的节点 ID
};

} // namespace Timeline

#endif // TIMELINE_DELEGATE_H
