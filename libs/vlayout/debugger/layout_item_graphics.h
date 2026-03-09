#ifndef VLAYOUT_LAYOUT_ITEM_GRAPHICS_H
#define VLAYOUT_LAYOUT_ITEM_GRAPHICS_H

/**
 * @file layout_item_graphics.h
 * @brief 布局项图形项
 */

#include "../global.h"
#include "../layoutitem.h"
#include "../boxlayout.h"
#include "sandbox_item.h"

#include <QGraphicsObject>
#include <QColor>

namespace VLayout {

/**
 * @class LayoutItemGraphics
 * @brief 单个布局项的图形表示
 *
 * 不可拖动，但可以点击选中、显示信息。
 * 支持两种数据源：旧的 SandboxItem 或新的 LayoutItem 指针。
 */
class VLAYOUT_EXPORT LayoutItemGraphics : public QGraphicsObject
{
    Q_OBJECT

public:
    enum { Type = UserType + 101 };

    /// 使用 SandboxItem 构造（兼容旧接口）
    explicit LayoutItemGraphics(const SandboxItem& item, int index, QGraphicsItem* parent = nullptr);

    /// 使用 LayoutItem 指针构造（新接口）
    explicit LayoutItemGraphics(LayoutItem* layoutItem, int index, QGraphicsItem* parent = nullptr);

    ~LayoutItemGraphics() override = default;

    // ========== 数据源 ==========

    /// 获取 SandboxItem 数据（兼容旧接口）
    const SandboxItem& itemData() const { return m_item; }
    void setItemData(const SandboxItem& item);

    /// 获取 LayoutItem 指针（新接口）
    LayoutItem* layoutItem() const { return m_layoutItem; }
    void setLayoutItem(LayoutItem* item);

    /// 从 LayoutItem 同步显示数据到 SandboxItem
    void syncFromLayoutItem(BoxLayout::Direction direction, int crossSize);

    int itemIndex() const { return m_index; }
    void setItemIndex(int index) { m_index = index; }

    // ========== 几何 ==========

    void setRect(const QRectF& rect);
    QRectF rect() const { return m_rect; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    int type() const override { return Type; }

    // ========== 选中状态 ==========

    void setSelectedVisual(bool selected);
    bool isSelectedVisual() const { return m_isSelected; }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

signals:
    void clicked(int index);
    void doubleClicked(int index);

private:
    QColor itemColor() const;
    void drawBackground(QPainter* painter);
    void drawBorder(QPainter* painter);
    void drawLabel(QPainter* painter);
    void drawStatusIndicator(QPainter* painter);

    /// 获取项标识（优先从 LayoutItem，否则从 SandboxItem）
    QString itemId() const;

    SandboxItem m_item;         ///< 显示数据缓存
    LayoutItem* m_layoutItem = nullptr;  ///< 布局项引用（新接口）
    int m_index;
    QRectF m_rect;

    bool m_isHovered = false;
    bool m_isSelected = false;
};

} // namespace VLayout

#endif // VLAYOUT_LAYOUT_ITEM_GRAPHICS_H
