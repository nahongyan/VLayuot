#ifndef VLAYOUT_BOXLAYOUT_H
#define VLAYOUT_BOXLAYOUT_H

/**
 * @file boxlayout.h
 * @brief 盒式布局实现
 *
 * 提供 BoxLayout（盒式布局）及其便捷子类 HBoxLayout（水平布局）和
 * VBoxLayout（垂直布局）。布局算法与 Qt QBoxLayout 完全一致。
 */

#include "layoutitem.h"
#include <vector>

namespace VLayout {

// ============================================================================
// LayoutStruct - 布局计算内部结构
// ============================================================================

/**
 * @struct LayoutStruct
 * @brief 布局计算内部数据结构
 *
 * 此结构映射 Qt 内部的 QLayoutStruct，被 qGeomCalc 算法用于在子项之间分配空间。
 * 每个子项对应一个 LayoutStruct 实例。
 */
struct LayoutStruct
{
    int sizeHint = 0;       ///< 首选尺寸
    int minimumSize = 0;    ///< 最小可接受尺寸
    int maximumSize = 0;    ///< 最大可接受尺寸
    int stretch = 0;        ///< stretch 因子
    int spacing = 0;        ///< 此项之后的间距
    int pos = 0;            ///< 计算出的位置（输出）
    int size = 0;           ///< 计算出的尺寸（输出）
    bool expansive = false; ///< 是否可扩展
    bool empty = false;     ///< 是否为空/隐藏
    bool done = false;      ///< 计算完成标志

    /**
     * @brief 返回有效的首选尺寸
     *
     * 与 Qt 的 QLayoutStruct::smartSizeHint() 行为完全一致：
     * - stretch > 0 的子项：返回 minimumSize，使其在空间不足（场景2）时
     *   不参与等比收缩，直接保留最小尺寸；多余空间才按 stretch 因子分配。
     * - stretch == 0 的子项：返回 max(sizeHint, minimumSize)，即尊重最小尺寸。
     *
     * @return 有效首选尺寸
     */
    int smartSizeHint() const {
        return (stretch > 0) ? minimumSize : qMax(sizeHint, minimumSize);
    }

    /**
     * @brief 返回有效间距
     * @param defaultSpacer 默认间距值
     * @return 如果项为空返回 0，否则返回有效间距
     */
    int effectiveSpacer(int defaultSpacer) const {
        return empty ? 0 : (spacing > 0 ? spacing : defaultSpacer);
    }
};

// ============================================================================
// BoxLayout - 盒式布局
// ============================================================================

/**
 * @class BoxLayout
 * @brief 水平或垂直排列子项的盒式布局
 *
 * BoxLayout 是主要的布局类，将子项按行（水平）或列（垂直）排列。
 * 布局算法与 Qt 的 QBoxLayout 完全一致，确保行为一致。
 *
 * ## 布局算法
 *
 * 算法处理三种场景：
 *
 * 1. **空间 < minimumSize**: 子项按比例收缩，从最大项开始
 * 2. **minimumSize <= 空间 < sizeHint**: 子项等比例收缩，尊重最小尺寸
 * 3. **空间 >= sizeHint**: 额外空间按 stretch 因子分配，尊重最大尺寸
 *
 * ## 使用示例
 * \code
 * auto layout = std::make_shared<VLayout::HBoxLayout>();
 * layout->setContentsMargins(10, 10, 10, 10);
 * layout->setSpacing(8);
 *
 * layout->addItem(VLayout::createLabel("title", "Name:"));
 * layout->addItem(VLayout::createStretch());
 * layout->addItem(VLayout::createButton("btn", "OK"));
 *
 * layout->setGeometry(QRect(0, 0, 300, 40));
 * layout->activate();
 * \endcode
 */
class VLAYOUT_EXPORT BoxLayout : public Layout
{
public:
    /**
     * @enum Direction
     * @brief 盒式布局的方向
     */
    enum class Direction {
        LeftToRight,    ///< 水平布局，从左到右
        TopToBottom     ///< 垂直布局，从上到下
    };

    /**
     * @brief 构造盒式布局
     * @param direction 布局方向
     */
    explicit BoxLayout(Direction direction = Direction::LeftToRight);

    ~BoxLayout() override;

    // ========== 方向 ==========

    /**
     * @brief 设置布局方向
     * @param direction 新的布局方向
     */
    void setDirection(Direction direction);

    /**
     * @brief 返回布局方向
     * @return 当前布局方向
     */
    Direction direction() const;

    /**
     * @brief 返回此布局的类型
     * @return 水平布局返回 ItemType::HBox，垂直布局返回 ItemType::VBox
     */
    ItemType type() const override {
        return m_direction == Direction::LeftToRight ? ItemType::HBox : ItemType::VBox;
    }

    // ========== 尺寸协商 ==========

    QSize sizeHint() const override;
    QSize minimumSize() const override;
    QSize maximumSize() const override;
    Qt::Orientations expandingDirections() const override;

    // ========== 边距 ==========

    /**
     * @brief 设置内容边距
     * @param left 左边距
     * @param top 上边距
     * @param right 右边距
     * @param bottom 下边距
     */
    void setContentsMargins(int left, int top, int right, int bottom);

    /**
     * @brief 设置统一的内容边距
     * @param margins 四边统一边距
     */
    void setContentsMargins(int margins);

    /**
     * @brief 获取内容边距
     * @param left 输出左边距
     * @param top 输出上边距
     * @param right 输出右边距
     * @param bottom 输出下边距
     */
    void getContentsMargins(int* left, int* top, int* right, int* bottom) const;

    // ========== 工具方法 ==========

    /**
     * @brief 在可用空间内对齐矩形
     * @param available 可用空间
     * @param size 目标尺寸
     * @param alignment 对齐方式
     * @return 对齐后的矩形
     */
    static QRect alignedRect(const QRect& available, const QSize& size, Qt::Alignment alignment);

    /**
     * @brief 核心几何计算算法（公开接口，供调试器使用）
     *
     * 这是 Qt qlayoutengine.cpp 中 qGeomCalc 函数的直接移植，
     * 使用相同的定点算术和分配逻辑。
     *
     * @param chain 布局结构数组
     * @param start 起始索引
     * @param count 子项数量
     * @param pos 起始位置
     * @param space 可用空间
     * @param spacer 默认间距
     */
    static void calculateGeometry(std::vector<LayoutStruct>& chain,
                                  int start, int count,
                                  int pos, int space, int spacer);

    /**
     * @brief 使布局失效
     *
     * 重写以同时重置脏标志。
     */
    void invalidate() override {
        m_dirty = true;
        LayoutItem::invalidate();
    }

protected:
    /// 执行实际的布局计算
    void doLayout(const QRect& rect) override;

private:
    // ========== 私有成员变量 ==========

    Direction m_direction;

    // 缓存的布局数据
    mutable std::vector<LayoutStruct> m_layoutStructs;
    mutable QSize m_cachedSizeHint;
    mutable QSize m_cachedMinSize;
    mutable QSize m_cachedMaxSize;
    mutable Qt::Orientations m_cachedExpanding;
    mutable bool m_dirty = true;

    // 边距
    int m_leftMargin = 0;
    int m_topMargin = 0;
    int m_rightMargin = 0;
    int m_bottomMargin = 0;

    // ========== 私有方法 ==========

    /// 设置布局数据（计算 sizeHint、minSize、maxSize 等）
    void setupLayoutData() const;

    /// 判断是否为水平布局
    static bool isHorizontal(Direction dir) {
        return dir == Direction::LeftToRight;
    }
};

// ============================================================================
// HBoxLayout - 水平盒式布局
// ============================================================================

/**
 * @class HBoxLayout
 * @brief 水平盒式布局便捷类
 *
 * 继承 BoxLayout，固定为 LeftToRight 方向。
 */
class VLAYOUT_EXPORT HBoxLayout : public BoxLayout
{
public:
    HBoxLayout() : BoxLayout(Direction::LeftToRight) {}
};

// ============================================================================
// VBoxLayout - 垂直盒式布局
// ============================================================================

/**
 * @class VBoxLayout
 * @brief 垂直盒式布局便捷类
 *
 * 继承 BoxLayout，固定为 TopToBottom 方向。
 */
class VLAYOUT_EXPORT VBoxLayout : public BoxLayout
{
public:
    VBoxLayout() : BoxLayout(Direction::TopToBottom) {}
};

} // namespace VLayout

#endif // VLAYOUT_BOXLAYOUT_H
