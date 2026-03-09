#ifndef VLAYOUT_COMPONENT_H
#define VLAYOUT_COMPONENT_H

/**
 * @file component.h
 * @brief 组件接口和基类定义
 *
 * 定义了 IComponent（组件接口）和 AbstractComponent（抽象组件基类）。
 * 组件是可绘制的 UI 元素，用于在 Delegate 中实现自定义绘制。
 */

#include "vlayout/global.h"
#include <QString>
#include <QRect>
#include <QSize>
#include <QFont>
#include <QPainter>
#include <QVariant>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <memory>
#include <functional>
#include <unordered_map>

namespace VLayout {

// ============================================================================
// 常量定义
// ============================================================================

/// 默认最大尺寸，与 Qt 的 QWIDGETSIZE_MAX 对应
constexpr int DefaultMaxSize = 16777215;

// ============================================================================
// ComponentState - 组件状态标志
// ============================================================================

/**
 * @enum ComponentState
 * @brief 组件状态标志位
 *
 * 组件状态使用位标志表示，可以组合多个状态。
 */
enum class ComponentState {
    None = 0,           ///< 无状态
    Enabled = 1 << 0,   ///< 启用状态
    Visible = 1 << 1,   ///< 可见状态
    Hovered = 1 << 2,   ///< 悬停状态
    Pressed = 1 << 3,   ///< 按下状态
    Focused = 1 << 4,   ///< 聚焦状态
    Selected = 1 << 5,  ///< 选中状态
    Checked = 1 << 6,   ///< 勾选状态
    Disabled = 1 << 7   ///< 禁用状态
};

// ============================================================================
// 状态位运算符
// ============================================================================

/// 状态位或运算
inline ComponentState operator|(ComponentState a, ComponentState b) {
    return static_cast<ComponentState>(static_cast<int>(a) | static_cast<int>(b));
}

/// 状态位与运算
inline ComponentState operator&(ComponentState a, ComponentState b) {
    return static_cast<ComponentState>(static_cast<int>(a) & static_cast<int>(b));
}

/// 状态位异或运算
inline ComponentState operator^(ComponentState a, ComponentState b) {
    return static_cast<ComponentState>(static_cast<int>(a) ^ static_cast<int>(b));
}

/// 状态位或赋值运算
inline ComponentState& operator|=(ComponentState& a, ComponentState b) {
    a = a | b;
    return a;
}

/// 状态位与赋值运算
inline ComponentState& operator&=(ComponentState& a, ComponentState b) {
    a = a & b;
    return a;
}

/// 状态位异或赋值运算
inline ComponentState& operator^=(ComponentState& a, ComponentState b) {
    a = a ^ b;
    return a;
}

/// 状态位非运算
inline ComponentState operator~(ComponentState a) {
    return static_cast<ComponentState>(~static_cast<int>(a));
}

// ============================================================================
// ComponentContext - 组件上下文
// ============================================================================

/**
 * @struct ComponentContext
 * @brief 组件绘制和交互的上下文信息
 *
 * ComponentContext 提供了组件绘制和交互所需的所有信息，
 * 包括绑定的数据、绘制工具和当前状态。
 */
struct ComponentContext {
    /// 绑定到组件的绘制器
    QPainter* painter = nullptr;

    /// 组件边界矩形
    QRect bounds;

    /// 关联的模型索引
    QModelIndex index;

    /// 样式选项（指针，避免每次拷贝）
    const QStyleOptionViewItem* option = nullptr;

    /// 当前状态标志（ComponentState 位组合）
    int states = 0;

    // ========== 主题相关属性 ==========

    /// 背景颜色
    QColor backgroundColor;

    /// 文本颜色
    QColor textColor;

    /// 强调颜色（如选中高亮）
    QColor accentColor;

    /// 当前字体
    QFont font;

    // ========== 状态检查便捷方法 ==========

    /// 是否悬停
    bool isHovered() const { return states & static_cast<int>(ComponentState::Hovered); }

    /// 是否按下
    bool isPressed() const { return states & static_cast<int>(ComponentState::Pressed); }

    /// 是否选中
    bool isSelected() const { return states & static_cast<int>(ComponentState::Selected); }

    /// 是否勾选
    bool isChecked() const { return states & static_cast<int>(ComponentState::Checked); }

    /// 是否启用
    bool isEnabled() const { return states & static_cast<int>(ComponentState::Enabled); }
};

// ============================================================================
// IComponent - 组件接口
// ============================================================================

/**
 * @class IComponent
 * @brief 所有 UI 组件的抽象接口
 *
 * IComponent 定义了组件必须实现的基本接口，包括：
 * - 标识（id, type）
 * - 布局（sizeHint, geometry）
 * - 绘制（paint）
 * - 交互（hitTest, onClick）
 * - 状态（setState, states）
 * - 属性（setProperty, property）
 * - 数据绑定（setData, data）
 */
class VLAYOUT_EXPORT IComponent
{
public:
    virtual ~IComponent() = default;

    // ========== 基本信息 ==========

    /// 返回组件唯一标识符
    virtual QString id() const = 0;

    /// 返回组件类型名称
    virtual QString type() const = 0;

    // ========== 布局 ==========

    /// 返回首选尺寸
    virtual QSize sizeHint() const = 0;

    /// 返回最小尺寸
    virtual QSize minimumSize() const = 0;

    /// 返回最大尺寸
    virtual QSize maximumSize() const = 0;

    /// 设置几何位置
    virtual void setGeometry(const QRect& rect) = 0;

    /// 返回当前几何位置
    virtual QRect geometry() const = 0;

    // ========== 绘制 ==========

    /**
     * @brief 绘制组件
     * @param ctx 绘制上下文
     */
    virtual void paint(ComponentContext& ctx) = 0;

    // ========== 交互 ==========

    /**
     * @brief 点击测试
     * @param point 测试点（全局坐标）
     * @return 如果点在组件内返回 true
     */
    virtual bool hitTest(const QPoint& point) const = 0;

    /// 鼠标进入事件
    virtual void onMouseEnter() {}

    /// 鼠标离开事件
    virtual void onMouseLeave() {}

    /// 鼠标按下事件
    virtual void onMousePress(const QPoint& pos) { Q_UNUSED(pos) }

    /// 鼠标释放事件
    virtual void onMouseRelease(const QPoint& pos) { Q_UNUSED(pos) }

    /// 点击事件
    virtual void onClick(const QPoint& pos) { Q_UNUSED(pos) }

    // ========== 状态 ==========

    /// 设置状态
    virtual void setState(ComponentState state, bool on = true) = 0;

    /// 返回状态标志
    virtual int states() const = 0;

    /// 检查是否具有指定状态
    virtual bool hasState(ComponentState state) const = 0;

    // ========== 属性 ==========

    /// 设置属性
    virtual void setProperty(const QString& name, const QVariant& value) = 0;

    /// 获取属性
    virtual QVariant property(const QString& name, const QVariant& defaultVal = QVariant()) const = 0;

    // ========== 数据绑定 ==========

    /// 设置绑定数据
    virtual void setData(const QVariant& data) = 0;

    /// 获取绑定数据
    virtual QVariant data() const = 0;
};

// ============================================================================
// AbstractComponent - 抽象组件基类
// ============================================================================

/**
 * @class AbstractComponent
 * @brief 组件的抽象基类，提供默认实现
 *
 * AbstractComponent 实现了 IComponent 接口的通用功能，
 * 子类只需实现 type() 和 paint() 方法即可。
 *
 * ## 使用示例
 * \code
 * class MyComponent : public AbstractComponent {
 * public:
 *     MyComponent(const QString& id) : AbstractComponent(id) {}
 *
 *     QString type() const override { return "MyComponent"; }
 *
 *     void paint(ComponentContext& ctx) override {
 *         QPainter* p = ctx.painter;
 *         QRect r = geometry();
 *         p->drawText(r, Qt::AlignCenter, text());
 *     }
 *
 *     void setText(const QString& t) { setProperty("text", t); }
 *     QString text() const { return property("text").toString(); }
 * };
 * \endcode
 */
class VLAYOUT_EXPORT AbstractComponent : public IComponent
{
public:
    /**
     * @brief 构造抽象组件
     * @param id 组件唯一标识符
     */
    explicit AbstractComponent(const QString& id);

    ~AbstractComponent() override = default;

    // ========== IComponent 接口实现 ==========

    QString id() const override { return m_id; }

    QSize sizeHint() const override { return m_sizeHint; }
    QSize minimumSize() const override { return m_minimumSize; }
    QSize maximumSize() const override { return m_maximumSize; }
    void setGeometry(const QRect& rect) override { m_geometry = rect; }
    QRect geometry() const override { return m_geometry; }

    void setState(ComponentState state, bool on = true) override;
    int states() const override { return m_states; }
    bool hasState(ComponentState state) const { return m_states & static_cast<int>(state); }

    void setProperty(const QString& name, const QVariant& value) override;
    QVariant property(const QString& name, const QVariant& defaultVal = QVariant()) const override;

    /// 检查是否具有指定属性
    bool hasProperty(const QString& name) const { return m_properties.count(name) > 0; }

    void setData(const QVariant& data) override { m_data = data; }
    QVariant data() const override { return m_data; }

    bool hitTest(const QPoint& point) const override {
        return m_geometry.contains(point);
    }

    // ========== 便捷方法 ==========

    /// 是否可见
    bool isVisible() const { return m_states & static_cast<int>(ComponentState::Visible); }

    /// 是否勾选
    bool isChecked() const { return m_states & static_cast<int>(ComponentState::Checked); }

    /// 是否启用
    bool isEnabled() const { return m_states & static_cast<int>(ComponentState::Enabled); }

protected:
    // ========== 受保护成员变量 ==========

    QString m_id;                       ///< 组件标识符
    QRect m_geometry;                   ///< 当前几何位置
    QSize m_sizeHint = QSize(100, 30);  ///< 首选尺寸
    QSize m_minimumSize;                ///< 最小尺寸
    QSize m_maximumSize = QSize(DefaultMaxSize, DefaultMaxSize);  ///< 最大尺寸
    int m_states = static_cast<int>(ComponentState::Enabled) | static_cast<int>(ComponentState::Visible); ///< 状态标志
    std::unordered_map<QString, QVariant> m_properties;  ///< 自定义属性映射
    QVariant m_data;                    ///< 绑定数据
};

} // namespace VLayout

#endif // VLAYOUT_COMPONENT_H
