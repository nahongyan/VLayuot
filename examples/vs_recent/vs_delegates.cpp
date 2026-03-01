#include "vs_delegates.h"
#include "vs_components.h"
#include "vs_roles.h"
#include "vs_theme.h"

#include <QHelpEvent>
#include <QToolTip>
#include <QMouseEvent>
#include <QPolygonF>

namespace VS {

// ============================================================================
// ProjectItemDelegate
// ============================================================================

ProjectItemDelegate::ProjectItemDelegate(QObject* parent)
    : VLayout::DelegateController(parent)
{
    // 设置布局边距和间距（左边距留出缩进空间）
    setMargins(24, 8, 12, 8);
    setSpacing(8);

    // 添加组件：[图标] [项目信息(stretch)] [日期] [图钉]
    addItem<SolutionIconComponent>("icon", 36, Qt::AlignVCenter);
    addItem<ProjectInfoComponent>("info", -1);  // -1 = 弹性填充剩余空间
    addItem<DateComponent>("date", 120, Qt::AlignVCenter);
    addItem<PinComponent>("pin", 24, Qt::AlignVCenter);

    // 设置行高
    setRowHeight(52);

    // 数据绑定
    bindTo("icon")
        .property("iconType", VS::IconTypeRole)
        .property("name", VS::NameRole);

    bindTo("info")
        .property("name", VS::NameRole)
        .property("path", VS::PathRole);

    bindTo("date")
        .property("date", VS::DateRole);

    bindTo("pin")
        .property("pinned", VS::PinnedRole);

    // 点击事件：切换固定状态
    onClick("pin", [](const QModelIndex& index, VLayout::IComponent*) {
        toggleData(index, VS::PinnedRole);
    });
}

void ProjectItemDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    // 绘制背景
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    bool isHovered = option.state & QStyle::State_MouseOver;

    if (isSelected) {
        painter->fillRect(option.rect, Theme::bgSelected);
    } else if (isHovered) {
        painter->fillRect(option.rect, Theme::bgHover);
    }

    painter->restore();

    // 调用父类绘制组件
    DelegateController::paint(painter, option, index);
}

QSize ProjectItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(400, 52);
}

bool ProjectItemDelegate::helpEvent(QHelpEvent* event,
                                     QAbstractItemView* view,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index)
{
    if (event->type() == QEvent::ToolTip) {
        // 确保布局已应用到当前行
        applyAutoBindings(index);
        applyAutoLayout(option.rect);

        QString tip;

        // 遍历组件检测悬停（geometry 是相对于 option.rect 的绝对坐标）
        for (VLayout::IComponent* comp : components()) {
            if (comp && comp->geometry().contains(event->pos())) {
                QString compId = comp->id();

                if (compId == QStringLiteral("pin")) {
                    bool pinned = index.data(VS::PinnedRole).toBool();
                    tip = pinned ? tr("Unpin this project") : tr("Pin this project");
                }
                else if (compId == QStringLiteral("info")) {
                    QString name = index.data(VS::NameRole).toString();
                    QString path = index.data(VS::PathRole).toString();
                    tip = name + QStringLiteral("\n") + path;
                }
                else if (compId == QStringLiteral("icon")) {
                    tip = index.data(VS::PathRole).toString();
                }
                break;
            }
        }

        if (!tip.isEmpty()) {
            QToolTip::showText(event->globalPos(), tip);
            return true;
        }
    }
    return DelegateController::helpEvent(event, view, option, index);
}

// ============================================================================
// GroupHeaderDelegate
// ============================================================================

void GroupHeaderDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    painter->save();

    bool isHovered = option.state & QStyle::State_MouseOver;

    // 绘制背景
    if (isHovered) {
        painter->fillRect(option.rect, Theme::bgHover);
    } else {
        painter->fillRect(option.rect, Theme::bgSurface);
    }

    // 获取折叠状态
    bool collapsed = index.data(VS::CollapsedRole).toBool();

    // 绘制展开/折叠箭头
    QRect arrowRect(option.rect.left() + 8, option.rect.top(),
                    16, option.rect.height());

    painter->setPen(Theme::textSecond);

    // 绘制三角形箭头
    QPolygonF arrow;
    int cx = arrowRect.center().x();
    int cy = arrowRect.center().y();

    if (collapsed) {
        // 向右箭头 >
        arrow << QPointF(cx - 4, cy - 4)
              << QPointF(cx + 4, cy)
              << QPointF(cx - 4, cy + 4);
    } else {
        // 向下箭头 v
        arrow << QPointF(cx - 4, cy - 3)
              << QPointF(cx + 4, cy - 3)
              << QPointF(cx, cy + 4);
    }

    painter->setBrush(Theme::textSecond);
    painter->drawPolygon(arrow);

    // 绘制分组标题文本
    QFont font(QStringLiteral("Segoe UI"), 9);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Theme::textHeader);

    QString text = index.data(Qt::DisplayRole).toString();
    QRect textRect = option.rect.adjusted(28, 0, 0, 0);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);

    // 绘制底部分隔线
    int lineY = option.rect.bottom() - 1;
    painter->setPen(Theme::separator);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(option.rect.left(), lineY,
                       option.rect.right(), lineY);

    painter->restore();
}

QSize GroupHeaderDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(400, 28);
}

bool GroupHeaderDelegate::editorEvent(QEvent* event,
                                       QAbstractItemModel* model,
                                       const QStyleOptionViewItem& option,
                                       const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        // 检测点击是否在箭头区域
        QRect arrowRect(option.rect.left() + 8, option.rect.top(),
                        16, option.rect.height());

        if (arrowRect.contains(mouseEvent->pos())) {
            // 切换折叠状态
            bool collapsed = index.data(VS::CollapsedRole).toBool();
            model->setData(index, !collapsed, VS::CollapsedRole);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ============================================================================
// RecentProjectsDelegate
// ============================================================================

RecentProjectsDelegate::RecentProjectsDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_projectDelegate(new ProjectItemDelegate(this))
    , m_groupDelegate(new GroupHeaderDelegate(this))
{
    // 转发项目委托的悬停信号
    connect(m_projectDelegate, &ProjectItemDelegate::clickableHoverChanged,
            this, &RecentProjectsDelegate::clickableHoverChanged);
}

void RecentProjectsDelegate::paint(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    bool isGroup = index.data(VS::IsGroupRole).toBool();

    if (isGroup) {
        m_groupDelegate->paint(painter, option, index);
    } else {
        m_projectDelegate->paint(painter, option, index);
    }
}

QSize RecentProjectsDelegate::sizeHint(const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    bool isGroup = index.data(VS::IsGroupRole).toBool();

    if (isGroup) {
        return m_groupDelegate->sizeHint(option, index);
    } else {
        return m_projectDelegate->sizeHint(option, index);
    }
}

bool RecentProjectsDelegate::editorEvent(QEvent* event,
                                          QAbstractItemModel* model,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index)
{
    bool isGroup = index.data(VS::IsGroupRole).toBool();

    if (isGroup) {
        return m_groupDelegate->editorEvent(event, model, option, index);
    }

    return m_projectDelegate->editorEvent(event, model, option, index);
}

bool RecentProjectsDelegate::helpEvent(QHelpEvent* event,
                                        QAbstractItemView* view,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index)
{
    bool isGroup = index.data(VS::IsGroupRole).toBool();

    if (!isGroup) {
        return m_projectDelegate->helpEvent(event, view, option, index);
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

} // namespace VS
