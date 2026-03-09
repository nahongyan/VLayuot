#ifndef VLAYOUT_CONTAINER_GRAPHICS_ITEM_H
#define VLAYOUT_CONTAINER_GRAPHICS_ITEM_H

/**
 * @file container_graphics_item.h
 * @brief 可拖动的容器图形项
 */

#include "../global.h"

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QColor>

namespace VLayout {

/**
 * @class ContainerGraphicsItem
 * @brief 可拖动的布局容器图形项
 *
 * 表示布局的最外层容器，可以被用户拖动移动位置。
 */
class VLAYOUT_EXPORT ContainerGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    enum { Type = UserType + 100 };

    explicit ContainerGraphicsItem(QGraphicsItem* parent = nullptr);
    ~ContainerGraphicsItem() override = default;

    // ========== 几何属性 ==========

    void setSize(const QSize& size);
    QSize size() const { return m_size; }

    void setMargins(int left, int top, int right, int bottom);
    void getMargins(int* left, int* top, int* right, int* bottom) const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    int type() const override { return Type; }

    // ========== 外观 ==========

    void setBorderColor(const QColor& color);
    void setMarginColor(const QColor& color);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void drawMargins(QPainter* painter);
    void drawContentArea(QPainter* painter);
    void drawBorder(QPainter* painter);

    QSize m_size = QSize(400, 200);
    int m_leftMargin = 8;
    int m_topMargin = 8;
    int m_rightMargin = 8;
    int m_bottomMargin = 8;

    QColor m_borderColor = QColor(100, 100, 100);
    QColor m_marginColor = QColor(200, 50, 50, 60);
    QColor m_backgroundColor = QColor(255, 255, 255);

    bool m_isDragging = false;
    QPointF m_dragStartPos;
};

} // namespace VLayout

#endif // VLAYOUT_CONTAINER_GRAPHICS_ITEM_H
