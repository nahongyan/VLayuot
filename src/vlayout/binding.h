#ifndef VLAYOUT_BINDING_H
#define VLAYOUT_BINDING_H

/**
 * @file binding.h
 * @brief 声明式数据绑定系统
 *
 * 提供从 Model 到 Component 的声明式数据绑定。在 DelegateController 构造函数中
 * 通过 bindTo() 声明绑定，框架在每次 paint() 时自动执行。
 *
 * ## 使用示例
 * \code
 * bindTo("title").text(Qt::DisplayRole).color(QColor(232, 232, 240));
 * bindTo("badge").text(RoleStatus, [](const QVariant& v) { return statusText(v); })
 *                .color(RoleStatus, [](const QVariant& v) { return statusColor(v); });
 * bindTo("spacer").size(RoleDepth, [](const QVariant& v) { return QSize(v.toInt()*16, 0); });
 * \endcode
 */

#include "vlayout/component.h"
#include <QModelIndex>
#include <QVariant>
#include <QFont>
#include <QColor>
#include <QSize>
#include <functional>
#include <unordered_map>
#include <memory>

namespace VLayout {

// ============================================================================
// 类型定义
// ============================================================================

/// 条件判断器类型
using Condition = std::function<bool(const QVariant&)>;

// ============================================================================
// PropertyBinding - 属性绑定
// ============================================================================

/**
 * @class PropertyBinding
 * @brief 单个属性的绑定配置
 *
 * PropertyBinding 管理一个组件属性的绑定，包括：
 * - 数据源（Model Role）
 * - 值转换器
 * - 可见性/启用/选中条件
 */
class PropertyBinding
{
public:
    /**
     * @brief 绑定数据源到指定 Role
     * @param role Qt::ItemDataRole 或自定义 Role
     * @return 自身引用，支持链式调用
     */
    PropertyBinding& fromRole(int role) {
        m_role = role;
        m_hasRole = true;
        return *this;
    }

    /**
     * @brief 设置文本转换器
     * @param fn 将 QVariant 转换为 QString 的函数
     * @return 自身引用
     */
    PropertyBinding& withText(std::function<QString(const QVariant&)> fn) {
        m_converter = [fn](const QVariant& v) -> QVariant { return fn(v); };
        return *this;
    }

    /**
     * @brief 设置固定字体
     * @param font 要设置的字体
     * @return 自身引用
     */
    PropertyBinding& withFont(const QFont& font) {
        m_converter = [font](const QVariant&) -> QVariant { return QVariant::fromValue(font); };
        return *this;
    }

    /**
     * @brief 设置字体转换器
     * @param fn 将 QVariant 转换为 QFont 的函数
     * @return 自身引用
     */
    PropertyBinding& withFont(std::function<QFont(const QVariant&)> fn) {
        m_converter = [fn](const QVariant& v) -> QVariant { return QVariant::fromValue(fn(v)); };
        return *this;
    }

    /**
     * @brief 设置固定颜色
     * @param color 要设置的颜色
     * @return 自身引用
     */
    PropertyBinding& withColor(const QColor& color) {
        m_converter = [color](const QVariant&) -> QVariant { return QVariant::fromValue(color); };
        return *this;
    }

    /**
     * @brief 设置颜色转换器
     * @param fn 将 QVariant 转换为 QColor 的函数
     * @return 自身引用
     */
    PropertyBinding& withColor(std::function<QColor(const QVariant&)> fn) {
        m_converter = [fn](const QVariant& v) -> QVariant { return QVariant::fromValue(fn(v)); };
        return *this;
    }

    /**
     * @brief 设置尺寸转换器
     * @param fn 将 QVariant 转换为 QSize 的函数
     * @return 自身引用
     */
    PropertyBinding& withSize(std::function<QSize(const QVariant&)> fn) {
        m_converter = [fn](const QVariant& v) -> QVariant { return QVariant::fromValue(fn(v)); };
        return *this;
    }

    /**
     * @brief 设置整数值转换器
     * @param fn 将 QVariant 转换为 int 的函数
     * @return 自身引用
     */
    PropertyBinding& withValue(std::function<int(const QVariant&)> fn) {
        m_converter = [fn](const QVariant& v) -> QVariant { return fn(v); };
        return *this;
    }

    /**
     * @brief 设置通用转换器
     * @param fn 将 QVariant 转换为 QVariant 的函数
     * @return 自身引用
     */
    PropertyBinding& withConverter(std::function<QVariant(const QVariant&)> fn) {
        m_converter = fn;
        return *this;
    }

    /**
     * @brief 设置可见性条件
     * @param condition 条件函数
     * @return 自身引用
     */
    PropertyBinding& visibleWhen(Condition condition) {
        m_visibleCondition = condition;
        return *this;
    }

    /**
     * @brief 设置可见性条件：数据非空
     * @return 自身引用
     */
    PropertyBinding& visibleWhenNotEmpty() {
        m_visibleCondition = [](const QVariant& v) { return !v.toString().isEmpty(); };
        return *this;
    }

    /**
     * @brief 设置启用条件
     * @param condition 条件函数
     * @return 自身引用
     */
    PropertyBinding& enabledWhen(Condition condition) {
        m_enabledCondition = condition;
        return *this;
    }

    /**
     * @brief 设置选中条件
     * @param condition 条件函数
     * @return 自身引用
     */
    PropertyBinding& checkedWhen(Condition condition) {
        m_checkedCondition = condition;
        return *this;
    }

    /**
     * @brief 应用绑定到组件
     * @param component 目标组件
     * @param index 模型索引
     */
    void apply(IComponent* component, const QModelIndex& index) const
    {
        if (!component) return;

        // 获取数据
        QVariant data;
        if (m_hasRole) {
            data = index.data(m_role);
        }

        // 应用转换器
        QVariant finalValue = m_converter ? m_converter(data) : data;

        // 设置属性值
        if (finalValue.isValid()) {
            component->setProperty(m_propertyName, finalValue);
        }

        // 应用状态条件
        if (m_visibleCondition) {
            component->setState(ComponentState::Visible, m_visibleCondition(data));
        }

        if (m_enabledCondition) {
            component->setState(ComponentState::Enabled, m_enabledCondition(data));
        }

        if (m_checkedCondition) {
            component->setState(ComponentState::Checked, m_checkedCondition(data));
        }
    }

private:
    friend class ComponentBinding;

    QString m_propertyName;         ///< 属性名称
    int m_role = Qt::DisplayRole;   ///< 数据 Role
    bool m_hasRole = false;         ///< 是否绑定了 Role
    std::function<QVariant(const QVariant&)> m_converter;  ///< 值转换器
    Condition m_visibleCondition;   ///< 可见性条件
    Condition m_enabledCondition;   ///< 启用条件
    Condition m_checkedCondition;   ///< 选中条件
};

// ============================================================================
// ComponentBinding - 组件绑定
// ============================================================================

/**
 * @class ComponentBinding
 * @brief 组件级别的绑定配置
 *
 * ComponentBinding 管理一个组件的所有属性绑定，包括：
 * - 多个属性绑定（text, color, font 等）
 * - 组件级别的状态条件
 * - 点击事件回调
 */
class ComponentBinding
{
public:
    /**
     * @brief 构造组件绑定
     * @param componentId 组件标识符
     */
    explicit ComponentBinding(const QString& componentId)
        : m_componentId(componentId)
    {}

    /// 返回组件标识符
    const QString& componentId() const { return m_componentId; }

    /**
     * @brief 绑定 text 属性
     * @param role 数据 Role
     * @return 属性绑定引用
     */
    PropertyBinding& bindText(int role = Qt::DisplayRole) {
        auto& pb = m_propertyBindings["text"];
        pb.m_propertyName = "text";
        pb.fromRole(role);
        return pb;
    }

    /**
     * @brief 绑定任意属性
     * @param propName 属性名称
     * @param role 数据 Role
     * @return 属性绑定引用
     */
    PropertyBinding& bindProperty(const QString& propName, int role) {
        auto& pb = m_propertyBindings[propName];
        pb.m_propertyName = propName;
        pb.fromRole(role);
        return pb;
    }

    /**
     * @brief 设置组件级别可见性条件
     * @param role 数据 Role
     * @param condition 条件函数
     * @return 自身引用
     */
    ComponentBinding& visibleWhen(int role, Condition condition) {
        m_visibleRole = role;
        m_visibleCondition = condition;
        return *this;
    }

    /**
     * @brief 设置可见性条件：值大于 0
     * @param role 数据 Role
     * @return 自身引用
     */
    ComponentBinding& visibleWhenPositive(int role) {
        return visibleWhen(role, [](const QVariant& v) { return v.toInt() > 0; });
    }

    /**
     * @brief 设置可见性条件：字符串非空
     * @param role 数据 Role
     * @return 自身引用
     */
    ComponentBinding& visibleWhenNotEmpty(int role) {
        return visibleWhen(role, [](const QVariant& v) { return !v.toString().isEmpty(); });
    }

    /**
     * @brief 设置启用条件
     * @param role 数据 Role
     * @param condition 条件函数
     * @return 自身引用
     */
    ComponentBinding& enabledWhen(int role, Condition condition) {
        m_enabledRole = role;
        m_enabledCondition = condition;
        return *this;
    }

    /**
     * @brief 设置选中条件
     * @param role 数据 Role
     * @param condition 条件函数
     * @return 自身引用
     */
    ComponentBinding& checkedWhen(int role, Condition condition) {
        m_checkedRole = role;
        m_checkedCondition = condition;
        return *this;
    }

    /**
     * @brief 设置选中条件：值为 true
     * @param role 数据 Role
     * @return 自身引用
     */
    ComponentBinding& checkedWhenTrue(int role) {
        return checkedWhen(role, [](const QVariant& v) { return v.toBool(); });
    }

    /**
     * @brief 设置点击事件回调
     * @param callback 回调函数
     * @return 自身引用
     */
    ComponentBinding& onClick(std::function<void(const QModelIndex&, IComponent*)> callback) {
        m_clickCallback = callback;
        return *this;
    }

    /**
     * @brief 应用所有绑定到组件
     * @param component 目标组件
     * @param index 模型索引
     */
    void apply(IComponent* component, const QModelIndex& index) const
    {
        if (!component) return;

        // 应用所有属性绑定
        for (const auto& pair : m_propertyBindings) {
            pair.second.apply(component, index);
        }

        // 应用组件级别状态条件
        if (m_visibleCondition) {
            QVariant data = index.data(m_visibleRole);
            component->setState(ComponentState::Visible, m_visibleCondition(data));
        }

        if (m_enabledCondition) {
            QVariant data = index.data(m_enabledRole);
            component->setState(ComponentState::Enabled, m_enabledCondition(data));
        }

        if (m_checkedCondition) {
            QVariant data = index.data(m_checkedRole);
            component->setState(ComponentState::Checked, m_checkedCondition(data));
        }
    }

    /// 返回点击回调
    const std::function<void(const QModelIndex&, IComponent*)>& clickCallback() const {
        return m_clickCallback;
    }

private:
    QString m_componentId;  ///< 组件标识符
    std::unordered_map<QString, PropertyBinding> m_propertyBindings;  ///< 属性绑定映射

    int m_visibleRole = -1;         ///< 可见性 Role
    Condition m_visibleCondition;   ///< 可见性条件

    int m_enabledRole = -1;         ///< 启用 Role
    Condition m_enabledCondition;   ///< 启用条件

    int m_checkedRole = -1;         ///< 选中 Role
    Condition m_checkedCondition;   ///< 选中条件

    std::function<void(const QModelIndex&, IComponent*)> m_clickCallback;  ///< 点击回调
};

// ============================================================================
// BindingBuilder - 流式绑定构建器
// ============================================================================

/**
 * @class BindingBuilder
 * @brief 流式 API 绑定构建器
 *
 * BindingBuilder 提供流式 API 来构建组件绑定，
 * 使代码更简洁、可读性更强。
 *
 * ## 使用示例
 * \code
 * bindTo("title")
 *     .display()
 *     .boldFont(12)
 *     .color(QColor(255, 255, 255));
 *
 * bindTo("status")
 *     .text(RoleStatus, [](const QVariant& v) { return v.toString(); })
 *     .visibleWhenNotEmpty(RoleStatus);
 * \endcode
 */
class BindingBuilder
{
public:
    /**
     * @brief 构建绑定构建器
     * @param componentId 组件标识符
     */
    explicit BindingBuilder(const QString& componentId)
        : m_binding(std::make_shared<ComponentBinding>(componentId))
    {}

    /// 构建并返回组件绑定
    std::shared_ptr<ComponentBinding> build() { return m_binding; }

    /**
     * @brief 绑定到 DisplayRole
     * @return 自身引用
     */
    BindingBuilder& display() {
        m_binding->bindText(Qt::DisplayRole);
        return *this;
    }

    /**
     * @brief 绑定到指定 Role 的文本
     * @param role 数据 Role
     * @return 自身引用
     */
    BindingBuilder& text(int role) {
        m_binding->bindText(role);
        return *this;
    }

    /**
     * @brief 绑定到指定 Role 的文本（带转换器）
     * @param role 数据 Role
     * @param converter 文本转换器
     * @return 自身引用
     */
    BindingBuilder& text(int role, std::function<QString(const QVariant&)> converter) {
        m_binding->bindProperty("text", role).withText(converter);
        return *this;
    }

    /**
     * @brief 绑定到 UserRole + offset
     * @param offset Role 偏移量
     * @return 自身引用
     */
    BindingBuilder& userRole(int offset = 0) {
        return text(Qt::UserRole + offset);
    }

    /**
     * @brief 绑定任意属性
     * @param name 属性名称
     * @param role 数据 Role
     * @return 自身引用
     */
    BindingBuilder& property(const QString& name, int role) {
        m_binding->bindProperty(name, role);
        return *this;
    }

    /**
     * @brief 绑定任意属性（带转换器）
     * @param name 属性名称
     * @param role 数据 Role
     * @param converter 值转换器
     * @return 自身引用
     */
    BindingBuilder& property(const QString& name, int role,
                             std::function<QVariant(const QVariant&)> converter) {
        m_binding->bindProperty(name, role).withConverter(converter);
        return *this;
    }

    /**
     * @brief 绑定 sizeHint 属性
     * @param role 数据 Role
     * @param fn 尺寸转换器
     * @return 自身引用
     */
    BindingBuilder& size(int role, std::function<QSize(const QVariant&)> fn) {
        m_binding->bindProperty("sizeHint", role).withSize(fn);
        return *this;
    }

    /**
     * @brief 设置固定字体
     * @param f 字体
     * @return 自身引用
     */
    BindingBuilder& font(const QFont& f) {
        m_binding->bindProperty("font", -1).withFont(f);
        return *this;
    }

    /**
     * @brief 绑定字体属性（带转换器）
     * @param role 数据 Role
     * @param fn 字体转换器
     * @return 自身引用
     */
    BindingBuilder& font(int role, std::function<QFont(const QVariant&)> fn) {
        m_binding->bindProperty("font", role).withFont(fn);
        return *this;
    }

    /**
     * @brief 设置粗体字体
     * @param pointSize 字号
     * @return 自身引用
     */
    BindingBuilder& boldFont(int pointSize = 11) {
        return font(-1, [pointSize](const QVariant&) {
            QFont f;
            f.setBold(true);
            f.setPointSize(pointSize);
            return f;
        });
    }

    /**
     * @brief 设置固定颜色
     * @param c 颜色
     * @return 自身引用
     */
    BindingBuilder& color(const QColor& c) {
        m_binding->bindProperty("color", -1).withColor(c);
        return *this;
    }

    /**
     * @brief 绑定颜色属性（带转换器）
     * @param role 数据 Role
     * @param fn 颜色转换器
     * @return 自身引用
     */
    BindingBuilder& color(int role, std::function<QColor(const QVariant&)> fn) {
        m_binding->bindProperty("color", role).withColor(fn);
        return *this;
    }

    /**
     * @brief 设置条件颜色
     * @param role 数据 Role
     * @param defaultColor 默认颜色
     * @param highlightColor 高亮颜色
     * @param condition 条件函数
     * @return 自身引用
     */
    BindingBuilder& conditionalColor(int role,
                                     const QColor& defaultColor,
                                     const QColor& highlightColor,
                                     std::function<bool(const QVariant&)> condition) {
        m_binding->bindProperty("color", role).withColor(
            [defaultColor, highlightColor, condition](const QVariant& v) {
                return condition(v) ? highlightColor : defaultColor;
            });
        return *this;
    }

    /**
     * @brief 设置可见性条件
     * @param role 数据 Role
     * @param condition 条件函数
     * @return 自身引用
     */
    BindingBuilder& visibleWhen(int role, Condition condition) {
        m_binding->visibleWhen(role, condition);
        return *this;
    }

    /**
     * @brief 设置可见性条件：值大于 0
     * @param role 数据 Role
     * @return 自身引用
     */
    BindingBuilder& visibleWhenPositive(int role) {
        m_binding->visibleWhenPositive(role);
        return *this;
    }

    /**
     * @brief 设置可见性条件：字符串非空
     * @param role 数据 Role
     * @return 自身引用
     */
    BindingBuilder& visibleWhenNotEmpty(int role) {
        m_binding->visibleWhenNotEmpty(role);
        return *this;
    }

    /**
     * @brief 设置选中条件：值为 true
     * @param role 数据 Role
     * @return 自身引用
     */
    BindingBuilder& checkedWhenTrue(int role) {
        m_binding->checkedWhenTrue(role);
        return *this;
    }

    /**
     * @brief 设置点击事件回调
     * @param callback 回调函数
     * @return 自身引用
     */
    BindingBuilder& onClick(std::function<void(const QModelIndex&, IComponent*)> callback) {
        m_binding->onClick(callback);
        return *this;
    }

private:
    std::shared_ptr<ComponentBinding> m_binding;  ///< 组件绑定
};

} // namespace VLayout

#endif // VLAYOUT_BINDING_H
