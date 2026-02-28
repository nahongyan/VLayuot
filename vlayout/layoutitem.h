#ifndef VLAYOUT_LAYOUTITEM_H
#define VLAYOUT_LAYOUTITEM_H

/**
 * @file layoutitem.h
 * @brief 布局项基类定义
 *
 * 定义了 LayoutItem（布局项基类）和 Layout（布局容器基类）两个核心类。
 * 所有布局项（WidgetItem、SpacerItem、Layout 等）都继承自 LayoutItem。
 */

#include "global.h"
#include <QRect>
#include <QSize>
#include <Qt>
#include <memory>
#include <vector>
#include <climits>

namespace VLayout {

// ============================================================================
// 常量定义
// ============================================================================

/// 最大尺寸常量，与 Qt 的 QWIDGETSIZE_MAX 行为一致
/// 使用 INT_MAX/256 以避免布局计算中的溢出问题
constexpr int SizeMax = INT_MAX / 256;

// ============================================================================
// 前向声明
// ============================================================================

class VLAYOUT_EXPORT Layout;

// ============================================================================
// ItemType - 布局项类型枚举
// ============================================================================

/**
 * @enum ItemType
 * @brief 布局项类型标识
 *
 * 用于运行时类型识别，避免 dynamic_cast 的 RTTI 开销。
 */
enum class ItemType {
    Widget,     ///< 虚拟控件项（WidgetItem）
    HBox,       ///< 水平盒式布局（HBoxLayout）
    VBox,       ///< 垂直盒式布局（VBoxLayout）
    Spacer      ///< 间隔项（SpacerItem）
};

// ============================================================================
// LayoutItem - 布局项基类
// ============================================================================

/**
 * @class LayoutItem
 * @brief 所有布局项的抽象基类
 *
 * LayoutItem 是所有布局项的抽象基类，包括控件项、布局和间隔项。
 * 它提供了尺寸协商和几何管理的通用接口。
 *
 * ## 尺寸协商机制
 * 布局算法通过三个关键方法进行尺寸协商：
 * - sizeHint(): 项的首选尺寸
 * - minimumSize(): 项可接受的最小尺寸
 * - maximumSize(): 项可接受的最大尺寸
 *
 * 布局算法使用这些值以及 stretch 因子和对齐方式来确定最终几何位置，
 * 在调用 activate() 后可通过 finalRect() 获取。
 *
 * ## 生命周期
 * - 通过 shared_ptr 管理生命周期
 * - 添加到 Layout 时自动设置父子关系
 * - 销毁时自动从父布局中移除
 */
class VLAYOUT_EXPORT LayoutItem
{
public:
    LayoutItem();
    virtual ~LayoutItem();

    // 禁用拷贝（布局项通过 shared_ptr 共享）
    LayoutItem(const LayoutItem&) = delete;
    LayoutItem& operator=(const LayoutItem&) = delete;

    // ========== 类型识别 ==========

    /**
     * @brief 返回此布局项的类型
     * @return 布局项类型枚举值
     */
    virtual ItemType type() const = 0;

    // ========== 尺寸协商 ==========

    /**
     * @brief 返回此布局项的首选尺寸
     *
     * sizeHint 是布局项的首选尺寸。布局会尽量为每个项分配其 sizeHint，
     * 除非受到可用空间或 minimum/maximum 尺寸的限制。
     *
     * @return 首选尺寸
     */
    virtual QSize sizeHint() const = 0;

    /**
     * @brief 返回此布局项的最小尺寸
     *
     * 布局永远不会给项分配小于其最小尺寸的空间。
     *
     * @return 最小尺寸，默认为 (0, 0)
     */
    virtual QSize minimumSize() const;

    /**
     * @brief 返回此布局项的最大尺寸
     *
     * 布局永远不会给项分配大于其最大尺寸的空间。
     *
     * @return 最大尺寸，默认为 SizeMax
     */
    virtual QSize maximumSize() const;

    /**
     * @brief 返回此布局项希望扩展的方向
     *
     * 布局使用此信息确定当有额外空间可用时，哪些项应该增长。
     *
     * @return 扩展方向的位或组合
     */
    virtual Qt::Orientations expandingDirections() const;

    // ========== 几何管理 ==========

    /**
     * @brief 设置此布局项的几何位置
     * @param rect 目标矩形区域
     */
    virtual void setGeometry(const QRect& rect);

    /**
     * @brief 返回此布局项的当前几何位置
     * @return 当前几何矩形
     */
    QRect geometry() const;

    /**
     * @brief 返回布局计算后的最终矩形
     *
     * 这是布局激活后项的实际位置和尺寸，用于绘制。
     *
     * @return 最终矩形
     */
    QRect finalRect() const;

    /**
     * @brief 设置最终矩形（布局内部使用）
     * @param rect 最终矩形
     */
    void setFinalRect(const QRect& rect);

    // ========== 布局属性 ==========

    /**
     * @brief 设置此布局项的对齐方式
     *
     * 当项的尺寸小于布局中的可用空间时使用。
     *
     * @param alignment Qt::Alignment 标志
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief 返回此布局项的对齐方式
     * @return 对齐标志
     */
    Qt::Alignment alignment() const;

    /**
     * @brief 设置此布局项的 stretch 因子
     *
     * - stretch = 0: 项不会增长，除非没有其他项可以增长
     * - stretch > 0: 项按 stretch 比例获得额外空间
     *
     * @param stretch stretch 因子
     */
    void setStretch(int stretch);

    /**
     * @brief 返回此布局项的 stretch 因子
     * @return stretch 因子
     */
    int stretch() const;

    /**
     * @brief 返回此布局项是否为空/隐藏
     *
     * 空项在布局中被特殊处理——它们不占用空间，
     * 但可能仍参与计算。
     *
     * @return 如果项为空返回 true
     */
    virtual bool isEmpty() const;

    /**
     * @brief 返回此布局项的父布局
     * @return 父布局指针，如果没有父布局则返回 nullptr
     */
    Layout* parentLayout() const;

    /**
     * @brief 使缓存的布局信息失效
     *
     * 当项的尺寸属性改变时调用，会向上传播到父布局。
     */
    virtual void invalidate();

    /**
     * @brief 返回布局是否有效（非脏）
     * @return 如果布局有效返回 true
     */
    bool isValid() const;

    // ========== 布局执行 ==========

    /**
     * @brief 执行实际的布局计算
     *
     * 子类必须实现此方法，为所有子项计算位置和尺寸。
     *
     * @param rect 可用矩形区域
     */
    virtual void doLayout(const QRect& rect);

protected:
    friend class Layout;

    /// 设置父布局（仅 Layout 类可调用）
    void setParentLayout(Layout* parent);

    // ========== 受保护成员变量 ==========

    QRect m_geometry;           ///< 当前几何位置（setGeometry 设置）
    QRect m_finalRect;          ///< 最终计算位置（布局激活后）
    Qt::Alignment m_alignment;  ///< 对齐方式
    int m_stretch = 0;          ///< stretch 因子
    Layout* m_parentLayout = nullptr;  ///< 父布局指针
    bool m_valid = false;       ///< 布局有效性标志
};

/// LayoutItem 共享指针类型
using LayoutItemPtr = std::shared_ptr<LayoutItem>;

/// LayoutItem 弱指针类型
using LayoutItemWeakPtr = std::weak_ptr<LayoutItem>;

// ============================================================================
// Layout - 布局容器基类
// ============================================================================

/**
 * @class Layout
 * @brief 包含子项的布局容器基类
 *
 * Layout 是所有布局的抽象基类，提供了管理子项和激活布局计算的通用接口。
 * 子类必须实现 doLayout() 来执行实际的几何计算。
 *
 * ## 子项管理
 * - 使用 shared_ptr 管理子项生命周期
 * - 子项添加时自动建立父子关系
 * - 支持添加、插入、移除和清空操作
 *
 * ## 布局激活
 * 1. 调用 setGeometry() 设置可用区域
 * 2. 调用 activate() 执行布局计算
 * 3. 各子项的 finalRect() 即可使用
 */
class VLAYOUT_EXPORT Layout : public LayoutItem
{
public:
    Layout();
    ~Layout() override;

    // ========== 子项管理 ==========

    /**
     * @brief 添加子项到此布局
     * @param item 要添加的子项
     */
    virtual void addItem(LayoutItemPtr item);

    /**
     * @brief 在指定位置插入子项
     * @param index 插入位置索引
     * @param item 要插入的子项
     */
    virtual void insertItem(int index, LayoutItemPtr item);

    /**
     * @brief 从此布局中移除子项
     * @param item 要移除的子项
     */
    virtual void removeItem(LayoutItemPtr item);

    /**
     * @brief 移除此布局中的所有子项
     */
    virtual void clear();

    /**
     * @brief 返回此布局中的子项数量
     * @return 子项数量
     */
    int count() const;

    /**
     * @brief 返回指定索引处的子项
     * @param index 子项索引
     * @return 子项指针，如果索引无效则返回 nullptr
     */
    LayoutItemPtr itemAt(int index) const;

    /**
     * @brief 返回指定子项的索引
     * @param item 子项指针
     * @return 子项索引，如果未找到则返回 -1
     */
    int indexOf(LayoutItemPtr item) const;

    // ========== 布局属性 ==========

    /**
     * @brief 设置子项之间的间距
     * @param spacing 间距像素值
     */
    void setSpacing(int spacing);

    /**
     * @brief 返回子项之间的间距
     * @return 间距像素值
     */
    int spacing() const;

    // ========== 布局激活 ==========

    /**
     * @brief 执行布局计算
     *
     * 在设置 geometry 后、使用 finalRect() 前调用。
     * 会调用 doLayout() 执行实际的布局计算。
     */
    void activate();

    /**
     * @brief 使布局失效并重新激活
     *
     * 等同于 invalidate() + activate()。
     */
    void update();

protected:
    /// 子项列表
    std::vector<LayoutItemPtr> m_items;
    /// 子项间距
    int m_spacing = 0;

    /// 子类实现具体的布局算法
    void doLayout(const QRect& rect) override = 0;
};

} // namespace VLayout

#endif // VLAYOUT_LAYOUTITEM_H
