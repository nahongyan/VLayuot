#ifndef VLAYOUT_WIDGETITEM_H
#define VLAYOUT_WIDGETITEM_H

/**
 * @file widgetitem.h
 * @brief 虚拟控件项定义
 *
 * WidgetItem 表示参与布局计算但不创建实际 QWidget 的虚拟控件。
 * 布局计算完成后，使用 finalRect() 获取位置和尺寸用于绘制。
 */

#include "layoutitem.h"
#include <QString>
#include <QVariant>
#include <functional>
#include <unordered_map>

#include "vlayout/global.h"  // For SizeMax constant

#include "component.h"  // For DefaultMaxSize

namespace VLayout {

// ============================================================================
// WidgetType - 虚拟控件类型
// ============================================================================

/**
 * @enum WidgetType
 * @brief 虚拟控件类型枚举
 */
enum class WidgetType {
    Label,          ///< 文本标签
    PushButton,     ///< 普通按钮
    ToolButton,     ///< 工具按钮
    CheckBox,       ///< 复选框
    RadioButton,    ///< 单选按钮
    LineEdit,       ///< 单行输入框
    ComboBox,       ///< 下拉框
    SpinBox,        ///< 整数调节框
    DoubleSpinBox,  ///< 浮点数调节框
    Slider,         ///< 滑动条
    ProgressBar,    ///< 进度条
    GroupBox,       ///< 分组框
    Frame,          ///< 边框
    Custom          ///< 自定义控件
};

// ============================================================================
// 回调类型定义
// ============================================================================

/// 点击事件回调类型
using ClickCallback = std::function<void(const QString& id)>;

/// 值变更事件回调类型
using ValueChangedCallback = std::function<void(const QString& id, const QVariant& value)>;

// ============================================================================
// WidgetItem - 虚拟控件项
// ============================================================================

/**
 * @class WidgetItem
 * @brief 表示布局中的虚拟控件
 *
 * WidgetItem 表示参与布局计算但不创建实际 QWidget 的虚拟控件。
 * 布局计算完成后，使用 finalRect() 获取位置和尺寸用于绘制。
 *
 * ## 主要特性
 * - 尺寸提示和约束（sizeHint, minimumSize, maximumSize）
 * - 状态管理（enabled, visible, checked, hovered, pressed, focused）
 * - 文本和值属性
 * - 通过 setProperty() 支持自定义属性
 * - 点击和值变更事件回调
 *
 * ## 使用示例
 * \code
 * auto button = VLayout::createButton("saveBtn", "Save");
 * button->setMinimumSize(QSize(80, 30));
 * button->setClickCallback([](const QString& id) {
 *     qDebug() << "Clicked:" << id;
 * });
 * layout->addItem(button);
 *
 * // 布局激活后：
 * QRect rect = button->finalRect();
 * painter->drawText(rect, Qt::AlignCenter, button->text());
 * \endcode
 */
class VLAYOUT_EXPORT WidgetItem : public LayoutItem
{
public:
    /**
     * @brief 构造虚拟控件项
     * @param id 唯一标识符
     * @param type 控件类型
     */
    WidgetItem(const QString& id, WidgetType type = WidgetType::Custom);

    /**
     * @brief 从类型名称字符串构造虚拟控件项
     * @param id 唯一标识符
     * @param typeName 类型名称（如 "QPushButton", "QLabel"）
     */
    WidgetItem(const QString& id, const QString& typeName);

    ~WidgetItem() override;

    ItemType type() const override { return ItemType::Widget; }

    // ========== 标识 ==========

    /**
     * @brief 返回此控件的唯一标识符
     * @return 标识符字符串
     */
    QString id() const { return m_id; }

    /**
     * @brief 设置此控件的唯一标识符
     * @param id 新标识符
     */
    void setId(const QString& id);

    /**
     * @brief 返回控件类型
     * @return 控件类型枚举
     */
    WidgetType widgetType() const { return m_widgetType; }

    /**
     * @brief 返回控件类型字符串（如 "QPushButton"）
     * @return 类型名称字符串
     */
    QString widgetTypeName() const;

    // ========== 尺寸协商 ==========

    QSize sizeHint() const override;
    QSize minimumSize() const override;
    QSize maximumSize() const override;

    /**
     * @brief 设置首选尺寸
     * @param size 首选尺寸
     */
    void setSizeHint(const QSize& size);

    /**
     * @brief 设置最小尺寸
     * @param size 最小尺寸
     */
    void setMinimumSize(const QSize& size);

    /**
     * @brief 设置最大尺寸
     * @param size 最大尺寸
     */
    void setMaximumSize(const QSize& size);

    /**
     * @brief 设置固定尺寸（同时设置最小和最大尺寸）
     * @param size 固定尺寸
     */
    void setFixedSize(const QSize& size);

    /**
     * @brief 设置最小宽度
     * @param width 最小宽度
     */
    void setMinimumWidth(int width);

    /**
     * @brief 设置最小高度
     * @param height 最小高度
     */
    void setMinimumHeight(int height);

    /**
     * @brief 设置最大宽度
     * @param width 最大宽度
     */
    void setMaximumWidth(int width);

    /**
     * @brief 设置最大高度
     * @param height 最大高度
     */
    void setMaximumHeight(int height);

    // ========== 状态 ==========

    /// 是否启用
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    /// 是否可见
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);

    /// 是否悬停
    bool isHovered() const { return m_hovered; }
    void setHovered(bool hovered);

    /// 是否按下
    bool isPressed() const { return m_pressed; }
    void setPressed(bool pressed);

    /// 是否聚焦
    bool isFocused() const { return m_focused; }
    void setFocused(bool focused);

    /// 是否选中
    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

    // ========== 内容 ==========

    /// 文本内容
    QString text() const { return m_text; }
    void setText(const QString& text);

    /// 值（用于进度条、滑块等）
    QVariant value() const { return m_value; }
    void setValue(const QVariant& value);

    // ========== 范围（用于滑块、调节框、进度条） ==========

    /// 最小值
    int minimum() const { return m_minimum; }
    void setMinimum(int min);

    /// 最大值
    int maximum() const { return m_maximum; }
    void setMaximum(int max);

    /// 设置范围
    void setRange(int min, int max);

    /// 单步值
    int singleStep() const { return m_singleStep; }
    void setSingleStep(int step);

    // ========== 自定义属性 ==========

    /**
     * @brief 获取自定义属性值
     * @param name 属性名
     * @param defaultValue 默认值（如果属性不存在）
     * @return 属性值或默认值
     */
    QVariant property(const QString& name, const QVariant& defaultValue = {}) const;

    /**
     * @brief 设置自定义属性值
     * @param name 属性名
     * @param value 属性值
     */
    void setProperty(const QString& name, const QVariant& value);

    // ========== 回调 ==========

    /// 获取点击回调
    ClickCallback clickCallback() const { return m_clickCallback; }
    void setClickCallback(ClickCallback callback);

    /// 获取值变更回调
    ValueChangedCallback valueChangedCallback() const { return m_valueChangedCallback; }
    void setValueChangedCallback(ValueChangedCallback callback);

    /**
     * @brief 模拟点击此控件
     */
    void click();

    // ========== 布局 ==========

    bool isEmpty() const override;

    /**
     * @brief 返回此控件是否需要重绘
     * @return 如果需要重绘返回 true
     */
    bool needsRepaint() const { return m_needsRepaint; }

    /**
     * @brief 设置是否需要重绘
     * @param needs 是否需要重绘（默认 true）
     */
    void setNeedsRepaint(bool needs = true);

private:
    /// 从类型名称解析控件类型
    static WidgetType widgetTypeFromName(const QString& name);

    // ========== 私有成员变量 ==========

    QString m_id;
    WidgetType m_widgetType;

    QSize m_sizeHint;
    QSize m_minimumSize;
    QSize m_maximumSize;

    bool m_enabled = true;
    bool m_visible = true;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_focused = false;
    bool m_checked = false;
    bool m_needsRepaint = true;

    QString m_text;
    QVariant m_value;
    int m_minimum = 0;
    int m_maximum = 100;
    int m_singleStep = 1;

    std::unordered_map<QString, QVariant> m_properties;

    ClickCallback m_clickCallback;
    ValueChangedCallback m_valueChangedCallback;
};

// ============================================================================
// 便捷工厂函数
// ============================================================================

/**
 * @brief 创建标签控件项
 * @param id 唯一标识符
 * @param text 标签文本
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createLabel(const QString& id, const QString& text = {})
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::Label);
    item->setText(text);
    return item;
}

/**
 * @brief 创建按钮控件项
 * @param id 唯一标识符
 * @param text 按钮文本
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createButton(const QString& id, const QString& text = {})
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::PushButton);
    item->setText(text);
    return item;
}

/**
 * @brief 创建复选框控件项
 * @param id 唯一标识符
 * @param text 复选框文本
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createCheckBox(const QString& id, const QString& text = {})
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::CheckBox);
    item->setText(text);
    return item;
}

/**
 * @brief 创建单选按钮控件项
 * @param id 唯一标识符
 * @param text 单选按钮文本
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createRadioButton(const QString& id, const QString& text = {})
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::RadioButton);
    item->setText(text);
    return item;
}

/**
 * @brief 创建单行输入框控件项
 * @param id 唯一标识符
 * @param text 初始文本
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createLineEdit(const QString& id, const QString& text = {})
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::LineEdit);
    item->setText(text);
    return item;
}

/**
 * @brief 创建下拉框控件项
 * @param id 唯一标识符
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createComboBox(const QString& id)
{
    return std::make_shared<WidgetItem>(id, WidgetType::ComboBox);
}

/**
 * @brief 创建进度条控件项
 * @param id 唯一标识符
 * @param value 初始值
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createProgressBar(const QString& id, int value = 0)
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::ProgressBar);
    item->setValue(value);
    return item;
}

/**
 * @brief 创建滑动条控件项
 * @param id 唯一标识符
 * @param value 初始值
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createSlider(const QString& id, int value = 0)
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::Slider);
    item->setValue(value);
    return item;
}

/**
 * @brief 创建整数调节框控件项
 * @param id 唯一标识符
 * @param value 初始值
 * @return 控件项共享指针
 */
inline std::shared_ptr<WidgetItem> createSpinBox(const QString& id, int value = 0)
{
    auto item = std::make_shared<WidgetItem>(id, WidgetType::SpinBox);
    item->setValue(value);
    return item;
}

} // namespace VLayout

#endif // VLAYOUT_WIDGETITEM_H
