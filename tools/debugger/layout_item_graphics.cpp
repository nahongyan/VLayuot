#include "layout_item_graphics.h"
#include <vlayout/boxlayout.h>
#include <vlayout/widgetitem.h>
#include <vlayout/spaceritem.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

namespace VLayout {

// ============================================================================
// 颜色常量
// ============================================================================

namespace {
    const QColor FixedColor(76, 175, 80, 120);      // 绿色
    const QColor StretchColor(33, 150, 243, 120);   // 蓝色
    const QColor SpacingColor(158, 158, 158, 120);  // 灰色
    const QColor CustomColor(255, 152, 0, 120);     // 橙色
    const QColor HBoxLayoutColor(156, 39, 176, 120); // 紫色 - 嵌套 HBoxLayout
    const QColor VBoxLayoutColor(103, 58, 183, 120); // 深紫色 - 嵌套 VBoxLayout
    const QColor SelectedColor(255, 235, 59, 180);  // 黄色高亮
    const QColor HoverColor(255, 255, 255, 80);     // 悬停效果
}

// ============================================================================
// LayoutItemGraphics 实现
// ============================================================================

LayoutItemGraphics::LayoutItemGraphics(const SandboxItem& item, int index, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_item(item)
    , m_index(index)
{
    setFlag(ItemIsMovable, false);  // 不可拖动
    setFlag(ItemIsSelectable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setZValue(10);
}

LayoutItemGraphics::LayoutItemGraphics(LayoutItem* layoutItem, int index, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , m_layoutItem(layoutItem)
    , m_index(index)
{
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setZValue(10);
}

// ========== 数据 ==========

void LayoutItemGraphics::setItemData(const SandboxItem& item)
{
    m_item = item;
    m_layoutItem = nullptr;  // 清除引用
    update();
}

void LayoutItemGraphics::setLayoutItem(LayoutItem* item)
{
    m_layoutItem = item;
    update();
}

void LayoutItemGraphics::syncFromLayoutItem(BoxLayout::Direction direction, int crossSize)
{
    if (!m_layoutItem) {
        return;
    }

    QRect finalRect = m_layoutItem->finalRect();
    QSize hint = m_layoutItem->sizeHint();
    QSize minSize = m_layoutItem->minimumSize();

    // 根据布局方向确定主方向和交叉方向
    bool isHorizontal = (direction == BoxLayout::Direction::LeftToRight);
    int mainSize = isHorizontal ? finalRect.width() : finalRect.height();
    int hintSize = isHorizontal ? hint.width() : hint.height();
    int minSizeValue = isHorizontal ? minSize.width() : minSize.height();

    // 更新 SandboxItem 缓存
    m_item.pos = isHorizontal ? finalRect.x() : finalRect.y();
    m_item.size = mainSize;
    m_item.crossSize = isHorizontal ? finalRect.height() : finalRect.width();

    // 从 LayoutItem 获取参数
    m_item.sizeHint = hintSize;
    m_item.minSize = minSizeValue;
    m_item.maxSize = isHorizontal ? m_layoutItem->maximumSize().width() : m_layoutItem->maximumSize().height();
    m_item.stretch = m_layoutItem->stretch();

    // 判断是否为 SpacerItem
    m_item.isSpacing = (m_layoutItem->type() == ItemType::Spacer);

    // 计算压缩和溢出状态
    m_item.isCompressed = (mainSize < hintSize);
    m_item.isOverflow = (mainSize < minSizeValue);

    // 获取 ID
    m_item.id = itemId();
}

QString LayoutItemGraphics::itemId() const
{
    if (m_layoutItem) {
        // 尝试从 WidgetItem 获取 ID
        if (m_layoutItem->type() == ItemType::Widget) {
            if (auto* widget = dynamic_cast<WidgetItem*>(m_layoutItem)) {
                return widget->id();
            }
        }
        // SpacerItem 或嵌套布局
        switch (m_layoutItem->type()) {
            case ItemType::Spacer:
                return QStringLiteral("spacer");
            case ItemType::HBox:
                return QStringLiteral("HBox");
            case ItemType::VBox:
                return QStringLiteral("VBox");
            default:
                return QStringLiteral("item");
        }
    }
    return m_item.id;
}

// ========== 几何 ==========

void LayoutItemGraphics::setRect(const QRectF& rect)
{
    if (m_rect != rect) {
        prepareGeometryChange();
        m_rect = rect;
        setPos(rect.topLeft());
        update();
    }
}

QRectF LayoutItemGraphics::boundingRect() const
{
    return QRectF(0, 0, m_rect.width(), m_rect.height());
}

QPainterPath LayoutItemGraphics::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

// ========== 选中状态 ==========

void LayoutItemGraphics::setSelectedVisual(bool selected)
{
    if (m_isSelected != selected) {
        m_isSelected = selected;
        update();
    }
}

// ========== 绘制 ==========

void LayoutItemGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // 选中高亮
    if (m_isSelected) {
        painter->fillRect(boundingRect(), SelectedColor);
    }

    // 背景
    drawBackground(painter);

    // 边框
    drawBorder(painter);

    // 标签
    drawLabel(painter);

    // 状态指示
    drawStatusIndicator(painter);
}

QColor LayoutItemGraphics::itemColor() const
{
    // 优先使用 LayoutItem 的类型信息
    if (m_layoutItem) {
        switch (m_layoutItem->type()) {
            case ItemType::Spacer:
                return SpacingColor;
            case ItemType::HBox:
                return HBoxLayoutColor;
            case ItemType::VBox:
                return VBoxLayoutColor;
            case ItemType::Widget:
                // WidgetItem 根据其属性判断颜色
                break;
            default:
                break;
        }
    }

    // 使用 SandboxItem 的信息（兼容旧接口）
    if (m_item.isSpacing) {
        return SpacingColor;
    }

    // 固定尺寸项
    if (m_item.minSize == m_item.maxSize) {
        return FixedColor;
    }

    // 弹性项
    if (m_item.stretch > 0) {
        return StretchColor;
    }

    return CustomColor;
}

void LayoutItemGraphics::drawBackground(QPainter* painter)
{
    QColor fillColor = itemColor();
    painter->fillRect(boundingRect(), fillColor);

    // 悬停效果
    if (m_isHovered) {
        painter->fillRect(boundingRect(), HoverColor);
    }
}

void LayoutItemGraphics::drawBorder(QPainter* painter)
{
    painter->save();

    if (m_isHovered) {
        painter->setPen(QPen(Qt::black, 2));
    } else {
        painter->setPen(QPen(Qt::darkGray, 1));
    }
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

    painter->restore();
}

void LayoutItemGraphics::drawLabel(QPainter* painter)
{
    // 间隔项不显示标签
    if (m_item.isSpacing || (m_layoutItem && m_layoutItem->type() == ItemType::Spacer)) {
        return;
    }

    painter->save();
    painter->setPen(Qt::black);
    QFont font("Microsoft YaHei", 9);
    painter->setFont(font);

    QString label;
    if (m_layoutItem) {
        // 从 LayoutItem 获取信息
        QString id = itemId();
        int mainSize = m_item.size;

        label = QString("%1\n%2px").arg(id).arg(mainSize);
        if (m_item.stretch > 0) {
            label += QString("\nstretch=%1").arg(m_item.stretch);
        }

        // 嵌套布局显示子项数量
        if (auto* layout = dynamic_cast<Layout*>(m_layoutItem)) {
            label += QString("\n[%1 items]").arg(layout->count());
        }
    } else {
        // 使用 SandboxItem 数据
        label = QString("%1\n%2px").arg(m_item.id).arg(m_item.size);
        if (m_item.stretch > 0) {
            label += QString("\nstretch=%1").arg(m_item.stretch);
        }
    }

    painter->drawText(boundingRect(), Qt::AlignCenter, label);

    painter->restore();
}

void LayoutItemGraphics::drawStatusIndicator(QPainter* painter)
{
    painter->save();

    if (m_item.isOverflow) {
        painter->setPen(QPen(Qt::red, 2));
        painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));
    } else if (m_item.isCompressed) {
        painter->setPen(QPen(QColor(255, 152, 0), 2));
        painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));
    }

    painter->restore();
}

// ========== 悬停事件 ==========

void LayoutItemGraphics::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_isHovered = true;
    update();

    // 显示 Tooltip
    QString tooltip;
    if (m_layoutItem) {
        QRect r = m_layoutItem->finalRect();
        QSize hint = m_layoutItem->sizeHint();
        tooltip = QString("%1\nfinalRect: (%2,%3) %4x%5\nsizeHint: %6x%7, stretch: %8")
            .arg(itemId())
            .arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height())
            .arg(hint.width()).arg(hint.height())
            .arg(m_layoutItem->stretch());
    } else {
        tooltip = QString("%1\npos: %2, size: %3\nhint: %4, stretch: %5")
            .arg(m_item.id)
            .arg(m_item.pos).arg(m_item.size)
            .arg(m_item.sizeHint).arg(m_item.stretch);
    }
    setToolTip(tooltip);
}

void LayoutItemGraphics::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_isHovered = false;
    update();
}

// ========== 鼠标事件 ==========

void LayoutItemGraphics::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_index);
    }
    QGraphicsObject::mousePressEvent(event);
}

void LayoutItemGraphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked(m_index);
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

} // namespace VLayout
