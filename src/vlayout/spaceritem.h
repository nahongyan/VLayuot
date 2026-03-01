#ifndef VLAYOUT_SPACERITEM_H
#define VLAYOUT_SPACERITEM_H

/**
 * @file spaceritem.h
 * @brief 间隔项定义
 *
 * SpacerItem 用于在布局中添加空白空间，可以是固定尺寸或可扩展的。
 */

#include "layoutitem.h"
#include <QSizePolicy>

namespace VLayout {

// ============================================================================
// SpacerItem - 间隔项
// ============================================================================

/**
 * @class SpacerItem
 * @brief 表示布局中的空白空间
 *
 * SpacerItem 用于在布局中添加空白区域。它可以是：
 * - 固定尺寸：始终占用指定大小的空间
 * - 可扩展：占用剩余空间，按 stretch 因子分配
 *
 * ## 使用示例
 * \code
 * auto layout = std::make_shared<VLayout::HBoxLayout>();
 *
 * // 固定 20 像素间隔
 * layout->addItem(std::make_shared<VLayout::SpacerItem>(20, 0));
 *
 * // 可扩展间隔（占用剩余空间）
 * auto spacer = std::make_shared<VLayout::SpacerItem>(0, 0);
 * spacer->setExpanding(Qt::Horizontal);
 * layout->addItem(spacer);
 *
 * // 或使用便捷函数
 * layout->addItem(VLayout::createFixedSpacer(20, true));  // 20px 水平间隔
 * layout->addItem(VLayout::createExpandingSpacer());       // 可扩展间隔
 * \endcode
 */
class VLAYOUT_EXPORT SpacerItem : public LayoutItem
{
public:
    /**
     * @brief 构造间隔项
     * @param width 固定宽度（0 表示可扩展）
     * @param height 固定高度（0 表示可扩展）
     * @param hPolicy 水平尺寸策略
     * @param vPolicy 垂直尺寸策略
     */
    SpacerItem(int width, int height,
               QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
               QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

    ~SpacerItem() override;

    ItemType type() const override { return ItemType::Spacer; }

    QSize sizeHint() const override;
    QSize minimumSize() const override;
    QSize maximumSize() const override;
    Qt::Orientations expandingDirections() const override;
    bool isEmpty() const override;

    /**
     * @brief 更改间隔项的尺寸和策略
     * @param width 新宽度
     * @param height 新高度
     * @param hPolicy 新水平尺寸策略
     * @param vPolicy 新垂直尺寸策略
     */
    void changeSize(int width, int height,
                    QSizePolicy::Policy hPolicy = QSizePolicy::Minimum,
                    QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

    /**
     * @brief 设置是否在指定方向上扩展
     * @param orientation 方向（Qt::Horizontal 或 Qt::Vertical）
     * @param expanding 是否扩展
     */
    void setExpanding(Qt::Orientation orientation, bool expanding = true);

    /**
     * @brief 返回水平尺寸策略
     * @return 水平尺寸策略
     */
    QSizePolicy::Policy horizontalPolicy() const { return m_hPolicy; }

    /**
     * @brief 返回垂直尺寸策略
     * @return 垂直尺寸策略
     */
    QSizePolicy::Policy verticalPolicy() const { return m_vPolicy; }

private:
    /// 尺寸（宽度和高度）
    QSize m_size;
    /// 水平尺寸策略
    QSizePolicy::Policy m_hPolicy;
    /// 垂直尺寸策略
    QSizePolicy::Policy m_vPolicy;
};

// ============================================================================
// 便捷工厂函数
// ============================================================================

/**
 * @brief 创建固定尺寸间隔项
 * @param size 间隔像素值
 * @param horizontal true 为水平间隔，false 为垂直间隔
 * @return 间隔项的共享指针
 */
inline LayoutItemPtr createFixedSpacer(int size, bool horizontal = true)
{
    if (horizontal) {
        return std::make_shared<SpacerItem>(size, 0,
            QSizePolicy::Fixed, QSizePolicy::Minimum);
    }
    return std::make_shared<SpacerItem>(0, size,
        QSizePolicy::Minimum, QSizePolicy::Fixed);
}

/**
 * @brief 创建可扩展间隔项
 * @param stretch stretch 因子（0 = 默认，越大获得越多空间）
 * @param horizontal true 为水平扩展，false 为垂直扩展
 * @return 间隔项的共享指针
 *
 * @note 可扩展间隔项会占用布局中的剩余空间。
 *       多个可扩展间隔项按 stretch 比例分配空间。
 */
inline LayoutItemPtr createExpandingSpacer(int stretch = 0, bool horizontal = true)
{
    if (horizontal) {
        auto spacer = std::make_shared<SpacerItem>(0, 0,
            QSizePolicy::Expanding, QSizePolicy::Minimum);
        spacer->setStretch(stretch);
        return spacer;
    }
    auto spacer = std::make_shared<SpacerItem>(0, 0,
        QSizePolicy::Minimum, QSizePolicy::Expanding);
    spacer->setStretch(stretch);
    return spacer;
}

/**
 * @brief 创建可扩展间隔项的便捷别名
 * @return 水平可扩展间隔项
 */
inline LayoutItemPtr createStretch()
{
    return createExpandingSpacer(0, true);
}

} // namespace VLayout

#endif // VLAYOUT_SPACERITEM_H
