#include "vlayout/widgetitem.h"
#include <QHash>

namespace VLayout {

// ============================================================================
// WidgetItem 实现
// ============================================================================

WidgetItem::WidgetItem(const QString& id, WidgetType type)
    : m_id(id)
    , m_widgetType(type)
    , m_minimumSize(0, 0)
    , m_maximumSize(SizeMax, SizeMax)
{
}

WidgetItem::WidgetItem(const QString& id, const QString& typeName)
    : m_id(id)
    , m_widgetType(widgetTypeFromName(typeName))
    , m_minimumSize(0, 0)
    , m_maximumSize(SizeMax, SizeMax)
{
}

WidgetItem::~WidgetItem()
{
}

QString WidgetItem::widgetTypeName() const
{
    switch (m_widgetType) {
    case WidgetType::Label:       return QStringLiteral("QLabel");
    case WidgetType::PushButton:  return QStringLiteral("QPushButton");
    case WidgetType::ToolButton:  return QStringLiteral("QToolButton");
    case WidgetType::CheckBox:    return QStringLiteral("QCheckBox");
    case WidgetType::RadioButton: return QStringLiteral("QRadioButton");
    case WidgetType::LineEdit:    return QStringLiteral("QLineEdit");
    case WidgetType::ComboBox:    return QStringLiteral("QComboBox");
    case WidgetType::SpinBox:     return QStringLiteral("QSpinBox");
    case WidgetType::DoubleSpinBox: return QStringLiteral("QDoubleSpinBox");
    case WidgetType::Slider:      return QStringLiteral("QSlider");
    case WidgetType::ProgressBar: return QStringLiteral("QProgressBar");
    case WidgetType::GroupBox:    return QStringLiteral("QGroupBox");
    case WidgetType::Frame:       return QStringLiteral("QFrame");
    case WidgetType::Custom:
    default:
        return QStringLiteral("Custom");
    }
}

void WidgetItem::setId(const QString& id)
{
    m_id = id;
}

// ============================================================================
// 尺寸协商
// ============================================================================

QSize WidgetItem::sizeHint() const
{
    if (m_sizeHint.isValid()) {
        return m_sizeHint;
    }

    // 根据控件类型返回默认尺寸
    switch (m_widgetType) {
    case WidgetType::Label:
        return QSize(100, 20);
    case WidgetType::PushButton:
        return QSize(80, 28);
    case WidgetType::ToolButton:
        return QSize(28, 28);
    case WidgetType::CheckBox:
    case WidgetType::RadioButton:
        return QSize(100, 20);
    case WidgetType::LineEdit:
        return QSize(150, 24);
    case WidgetType::ComboBox:
        return QSize(120, 24);
    case WidgetType::SpinBox:
    case WidgetType::DoubleSpinBox:
        return QSize(80, 24);
    case WidgetType::Slider:
        return QSize(100, 20);
    case WidgetType::ProgressBar:
        return QSize(100, 20);
    case WidgetType::GroupBox:
        return QSize(200, 100);
    case WidgetType::Frame:
        return QSize(100, 20);
    case WidgetType::Custom:
    default:
        return QSize(100, 30);
    }
}

QSize WidgetItem::minimumSize() const
{
    return m_minimumSize;
}

QSize WidgetItem::maximumSize() const
{
    return m_maximumSize;
}

void WidgetItem::setSizeHint(const QSize& size)
{
    if (m_sizeHint != size) {
        m_sizeHint = size;
        invalidate();
    }
}

void WidgetItem::setMinimumSize(const QSize& size)
{
    if (m_minimumSize != size) {
        m_minimumSize = size;
        invalidate();
    }
}

void WidgetItem::setMaximumSize(const QSize& size)
{
    if (m_maximumSize != size) {
        m_maximumSize = size;
        invalidate();
    }
}

void WidgetItem::setFixedSize(const QSize& size)
{
    m_minimumSize = size;
    m_maximumSize = size;
    invalidate();
}

void WidgetItem::setMinimumWidth(int width)
{
    m_minimumSize.setWidth(width);
    invalidate();
}

void WidgetItem::setMinimumHeight(int height)
{
    m_minimumSize.setHeight(height);
    invalidate();
}

void WidgetItem::setMaximumWidth(int width)
{
    m_maximumSize.setWidth(width);
    invalidate();
}

void WidgetItem::setMaximumHeight(int height)
{
    m_maximumSize.setHeight(height);
    invalidate();
}

// ============================================================================
// 状态管理
// ============================================================================

void WidgetItem::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_needsRepaint = true;
    }
}

void WidgetItem::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        m_needsRepaint = true;
        invalidate();
    }
}

void WidgetItem::setHovered(bool hovered)
{
    if (m_hovered != hovered) {
        m_hovered = hovered;
        m_needsRepaint = true;
    }
}

void WidgetItem::setPressed(bool pressed)
{
    if (m_pressed != pressed) {
        m_pressed = pressed;
        m_needsRepaint = true;
    }
}

void WidgetItem::setFocused(bool focused)
{
    if (m_focused != focused) {
        m_focused = focused;
        m_needsRepaint = true;
    }
}

void WidgetItem::setChecked(bool checked)
{
    if (m_checked != checked) {
        m_checked = checked;
        m_needsRepaint = true;

        // 触发值变更回调
        if (m_valueChangedCallback) {
            m_valueChangedCallback(m_id, m_checked);
        }
    }
}

// ============================================================================
// 内容管理
// ============================================================================

void WidgetItem::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        m_needsRepaint = true;
    }
}

void WidgetItem::setValue(const QVariant& value)
{
    if (m_value != value) {
        m_value = value;
        m_needsRepaint = true;

        // 触发值变更回调
        if (m_valueChangedCallback) {
            m_valueChangedCallback(m_id, m_value);
        }
    }
}

// ============================================================================
// 范围管理
// ============================================================================

void WidgetItem::setMinimum(int min)
{
    m_minimum = min;
    m_needsRepaint = true;
}

void WidgetItem::setMaximum(int max)
{
    m_maximum = max;
    m_needsRepaint = true;
}

void WidgetItem::setRange(int min, int max)
{
    m_minimum = min;
    m_maximum = max;
    m_needsRepaint = true;
}

void WidgetItem::setSingleStep(int step)
{
    m_singleStep = step;
}

// ============================================================================
// 自定义属性
// ============================================================================

QVariant WidgetItem::property(const QString& name, const QVariant& defaultValue) const
{
    auto it = m_properties.find(name);
    return (it != m_properties.end()) ? it->second : defaultValue;
}

void WidgetItem::setProperty(const QString& name, const QVariant& value)
{
    m_properties[name] = value;
}

// ============================================================================
// 回调管理
// ============================================================================

void WidgetItem::setClickCallback(ClickCallback callback)
{
    m_clickCallback = std::move(callback);
}

void WidgetItem::setValueChangedCallback(ValueChangedCallback callback)
{
    m_valueChangedCallback = std::move(callback);
}

void WidgetItem::click()
{
    // 触发点击回调
    if (m_clickCallback) {
        m_clickCallback(m_id);
    }
    m_needsRepaint = true;
}

// ============================================================================
// 布局相关
// ============================================================================

bool WidgetItem::isEmpty() const
{
    return !m_visible;
}

void WidgetItem::setNeedsRepaint(bool needs)
{
    m_needsRepaint = needs;
}

// ============================================================================
// 私有方法
// ============================================================================

WidgetType WidgetItem::widgetTypeFromName(const QString& name)
{
    // 使用静态哈希表实现 O(1) 查找，避免多次字符串比较
    static const QHash<QString, WidgetType> typeMap = {
        // 带Q前缀
        {QStringLiteral("QLabel"), WidgetType::Label},
        {QStringLiteral("QPushButton"), WidgetType::PushButton},
        {QStringLiteral("QToolButton"), WidgetType::ToolButton},
        {QStringLiteral("QCheckBox"), WidgetType::CheckBox},
        {QStringLiteral("QRadioButton"), WidgetType::RadioButton},
        {QStringLiteral("QLineEdit"), WidgetType::LineEdit},
        {QStringLiteral("QComboBox"), WidgetType::ComboBox},
        {QStringLiteral("QSpinBox"), WidgetType::SpinBox},
        {QStringLiteral("QDoubleSpinBox"), WidgetType::DoubleSpinBox},
        {QStringLiteral("QSlider"), WidgetType::Slider},
        {QStringLiteral("QProgressBar"), WidgetType::ProgressBar},
        {QStringLiteral("QGroupBox"), WidgetType::GroupBox},
        {QStringLiteral("QFrame"), WidgetType::Frame},
        // 不带Q前缀
        {QStringLiteral("Label"), WidgetType::Label},
        {QStringLiteral("PushButton"), WidgetType::PushButton},
        {QStringLiteral("ToolButton"), WidgetType::ToolButton},
        {QStringLiteral("CheckBox"), WidgetType::CheckBox},
        {QStringLiteral("RadioButton"), WidgetType::RadioButton},
        {QStringLiteral("LineEdit"), WidgetType::LineEdit},
        {QStringLiteral("ComboBox"), WidgetType::ComboBox},
        {QStringLiteral("SpinBox"), WidgetType::SpinBox},
        {QStringLiteral("DoubleSpinBox"), WidgetType::DoubleSpinBox},
        {QStringLiteral("Slider"), WidgetType::Slider},
        {QStringLiteral("ProgressBar"), WidgetType::ProgressBar},
        {QStringLiteral("GroupBox"), WidgetType::GroupBox},
        {QStringLiteral("Frame"), WidgetType::Frame}
    };

    auto it = typeMap.constFind(name);
    if (it != typeMap.constEnd()) {
        return it.value();
    }
    return WidgetType::Custom;
}

} // namespace VLayout
