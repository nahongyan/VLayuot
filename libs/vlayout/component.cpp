#include "vlayout/component.h"
#include <QApplication>
#include <QStyle>

namespace VLayout {

// ============================================================================
// Constants
// ============================================================================

namespace {
    constexpr int DefaultMaxSize = 16777215;
}

// ============================================================================
// AbstractComponent Implementation
// ============================================================================

AbstractComponent::AbstractComponent(const QString& id)
    : m_id(id)
    , m_sizeHint(100, 30)
    , m_minimumSize(0, 0)
    , m_maximumSize(DefaultMaxSize, DefaultMaxSize)
{
}

void AbstractComponent::setState(ComponentState state, bool on)
{
    int flag = static_cast<int>(state);
    if (on) {
        m_states |= flag;
    } else {
        m_states &= ~flag;
    }
}

void AbstractComponent::setProperty(const QString& name, const QVariant& value)
{
    m_properties[name] = value;

    // 统一 sizeHint：当绑定系统设置 "sizeHint" 属性时，自动同步到 m_sizeHint
    if (name == QLatin1String("sizeHint") && value.canConvert<QSize>()) {
        m_sizeHint = value.value<QSize>();
    }
}

QVariant AbstractComponent::property(const QString& name, const QVariant& defaultVal) const
{
    auto it = m_properties.find(name);
    return it != m_properties.end() ? it->second : defaultVal;
}

} // namespace VLayout
