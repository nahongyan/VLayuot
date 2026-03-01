#include "vlayout/spaceritem.h"

namespace VLayout {

// ============================================================================
// SpacerItem 实现
// ============================================================================

SpacerItem::SpacerItem(int width, int height,
                       QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
    : m_size(width, height)
    , m_hPolicy(hPolicy)
    , m_vPolicy(vPolicy)
{
}

SpacerItem::~SpacerItem()
{
}

QSize SpacerItem::sizeHint() const
{
    return m_size;
}

QSize SpacerItem::minimumSize() const
{
    // Fixed 策略方向返回固定尺寸，否则返回 0
    return QSize(
        m_hPolicy == QSizePolicy::Fixed ? m_size.width() : 0,
        m_vPolicy == QSizePolicy::Fixed ? m_size.height() : 0
    );
}

QSize SpacerItem::maximumSize() const
{
    // Fixed 策略方向返回固定尺寸，否则返回 SizeMax
    return QSize(
        m_hPolicy == QSizePolicy::Fixed ? m_size.width() : SizeMax,
        m_vPolicy == QSizePolicy::Fixed ? m_size.height() : SizeMax
    );
}

Qt::Orientations SpacerItem::expandingDirections() const
{
    Qt::Orientations result;

    // Expanding 或 MinimumExpanding 策略表示可以扩展
    if (m_hPolicy == QSizePolicy::Expanding || m_hPolicy == QSizePolicy::MinimumExpanding) {
        result |= Qt::Horizontal;
    }

    if (m_vPolicy == QSizePolicy::Expanding || m_vPolicy == QSizePolicy::MinimumExpanding) {
        result |= Qt::Vertical;
    }

    return result;
}

bool SpacerItem::isEmpty() const
{
    // 间隔项始终被视为"空"（不包含实际内容）
    return true;
}

void SpacerItem::changeSize(int width, int height,
                            QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
{
    m_size = QSize(width, height);
    m_hPolicy = hPolicy;
    m_vPolicy = vPolicy;
    invalidate();
}

void SpacerItem::setExpanding(Qt::Orientation orientation, bool expanding)
{
    if (orientation == Qt::Horizontal) {
        m_hPolicy = expanding ? QSizePolicy::Expanding : QSizePolicy::Minimum;
    } else {
        m_vPolicy = expanding ? QSizePolicy::Expanding : QSizePolicy::Minimum;
    }
    invalidate();
}

} // namespace VLayout
