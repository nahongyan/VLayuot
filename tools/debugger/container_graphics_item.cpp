#include "container_graphics_item.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

namespace VLayout {

// ============================================================================
// ContainerGraphicsItem 实现
// ============================================================================

ContainerGraphicsItem::ContainerGraphicsItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setFlag(ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setZValue(0);
}

// ========== 几何属性 ==========

void ContainerGraphicsItem::setSize(const QSize& size)
{
    if (m_size != size) {
        prepareGeometryChange();
        m_size = size;
        update();
    }
}

void ContainerGraphicsItem::setMargins(int left, int top, int right, int bottom)
{
    m_leftMargin = left;
    m_topMargin = top;
    m_rightMargin = right;
    m_bottomMargin = bottom;
    update();
}

void ContainerGraphicsItem::getMargins(int* left, int* top, int* right, int* bottom) const
{
    if (left) *left = m_leftMargin;
    if (top) *top = m_topMargin;
    if (right) *right = m_rightMargin;
    if (bottom) *bottom = m_bottomMargin;
}

QRectF ContainerGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, m_size.width(), m_size.height());
}

QPainterPath ContainerGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

// ========== 外观 ==========

void ContainerGraphicsItem::setBorderColor(const QColor& color)
{
    m_borderColor = color;
    update();
}

void ContainerGraphicsItem::setMarginColor(const QColor& color)
{
    m_marginColor = color;
    update();
}

// ========== 绘制 ==========

void ContainerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    painter->fillRect(boundingRect(), m_backgroundColor);

    // Margin 区域
    drawMargins(painter);

    // 边框
    drawBorder(painter);
}

void ContainerGraphicsItem::drawMargins(QPainter* painter)
{
    painter->save();
    painter->setOpacity(0.3);

    // 左 margin
    if (m_leftMargin > 0) {
        QRectF leftRect(0, 0, m_leftMargin, m_size.height());
        painter->fillRect(leftRect, m_marginColor);
    }

    // 右 margin
    if (m_rightMargin > 0) {
        QRectF rightRect(m_size.width() - m_rightMargin, 0, m_rightMargin, m_size.height());
        painter->fillRect(rightRect, m_marginColor);
    }

    // 上 margin
    if (m_topMargin > 0) {
        QRectF topRect(m_leftMargin, 0,
                       m_size.width() - m_leftMargin - m_rightMargin, m_topMargin);
        painter->fillRect(topRect, m_marginColor);
    }

    // 下 margin
    if (m_bottomMargin > 0) {
        QRectF bottomRect(m_leftMargin, m_size.height() - m_bottomMargin,
                          m_size.width() - m_leftMargin - m_rightMargin, m_bottomMargin);
        painter->fillRect(bottomRect, m_marginColor);
    }

    painter->restore();
}

void ContainerGraphicsItem::drawContentArea(QPainter* painter)
{
    Q_UNUSED(painter);
    // 内容区域不需要特别绘制，由子项负责
}

void ContainerGraphicsItem::drawBorder(QPainter* painter)
{
    painter->save();

    QPen pen(m_borderColor, isSelected() ? 2 : 1);
    if (isSelected()) {
        pen.setStyle(Qt::DashLine);
    }
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

    painter->restore();
}

// ========== 鼠标事件 ==========

void ContainerGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPos = pos();
    }
    QGraphicsObject::mousePressEvent(event);
}

void ContainerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isDragging) {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void ContainerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    m_isDragging = false;
    QGraphicsObject::mouseReleaseEvent(event);
}

} // namespace VLayout
