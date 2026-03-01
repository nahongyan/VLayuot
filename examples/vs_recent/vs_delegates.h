#ifndef VS_DELEGATES_H
#define VS_DELEGATES_H

/**
 * @file vs_delegates.h
 * @brief Delegate 类定义
 *
 * 定义了用于最近项目列表的委托类。
 */

#include "vlayout/framework.h"
#include <QStyledItemDelegate>

namespace VS {

/**
 * @class ProjectItemDelegate
 * @brief 项目行委托
 *
 * 使用 VLayout 框架绘制单个项目行。
 *
 * ## 布局
 * [图标(36px)] [项目信息(stretch)] [日期(100px)] [图钉(24px)]
 *
 * ## 功能
 * - 显示项目图标、名称、路径、日期、固定按钮
 * - 鼠标悬停时高亮
 * - 点击图钉切换固定状态
 * - 支持 tooltip（项目路径、固定提示）
 */
class ProjectItemDelegate : public VLayout::DelegateController
{
    Q_OBJECT

public:
    explicit ProjectItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;
};

/**
 * @class GroupHeaderDelegate
 * @brief 分组标题委托
 *
 * 绘制分组标题行（如"已固定"、"本月"、"更早"）。
 *
 * ## 特点
 * - 展开/折叠箭头（手绘）
 * - 粗体文本
 * - 悬停时高亮
 * - 底部分隔线
 */
class GroupHeaderDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;
};

/**
 * @class RecentProjectsDelegate
 * @brief 组合委托
 *
 * 根据 IsGroupRole 数据角色将绘制任务分派到适当的子委托。
 * - IsGroupRole == true  → GroupHeaderDelegate
 * - IsGroupRole == false → ProjectItemDelegate
 */
class RecentProjectsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RecentProjectsDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

    bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    /// 返回内部项目委托
    ProjectItemDelegate* projectDelegate() const { return m_projectDelegate; }

signals:
    /// 可点击组件悬停状态变化（用于手型光标）
    void clickableHoverChanged(const QString& componentId, bool hovered);

private:
    ProjectItemDelegate* m_projectDelegate;
    GroupHeaderDelegate* m_groupDelegate;
};

} // namespace VS

#endif // VS_DELEGATES_H
