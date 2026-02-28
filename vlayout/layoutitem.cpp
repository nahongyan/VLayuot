#include "vlayout/layoutitem.h"
#include <algorithm>

namespace VLayout {

// ============================================================================
// LayoutItem 实现
// ============================================================================

LayoutItem::LayoutItem()
{
}

LayoutItem::~LayoutItem()
{
}

QSize LayoutItem::minimumSize() const
{
    return QSize(0, 0);
}

QSize LayoutItem::maximumSize() const
{
    return QSize(SizeMax, SizeMax);
}

Qt::Orientations LayoutItem::expandingDirections() const
{
    return {};
}

void LayoutItem::setGeometry(const QRect& rect)
{
    m_geometry = rect;
    m_valid = true;
}

QRect LayoutItem::geometry() const
{
    return m_geometry;
}

QRect LayoutItem::finalRect() const
{
    return m_finalRect;
}

void LayoutItem::setFinalRect(const QRect& rect)
{
    m_finalRect = rect;
}

void LayoutItem::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
}

Qt::Alignment LayoutItem::alignment() const
{
    return m_alignment;
}

void LayoutItem::setStretch(int stretch)
{
    m_stretch = stretch;
}

int LayoutItem::stretch() const
{
    return m_stretch;
}

bool LayoutItem::isEmpty() const
{
    return false;
}

Layout* LayoutItem::parentLayout() const
{
    return m_parentLayout;
}

void LayoutItem::setParentLayout(Layout* parent)
{
    m_parentLayout = parent;
}

void LayoutItem::invalidate()
{
    m_valid = false;
    // 向上传播到父布局
    if (m_parentLayout) {
        m_parentLayout->invalidate();
    }
}

bool LayoutItem::isValid() const
{
    return m_valid;
}

void LayoutItem::doLayout(const QRect& rect)
{
    Q_UNUSED(rect);
    // 默认实现不做任何操作，子类重写
}

// ============================================================================
// Layout 实现
// ============================================================================

Layout::Layout()
    : LayoutItem()
{
}

Layout::~Layout()
{
    clear();
}

void Layout::addItem(LayoutItemPtr item)
{
    if (!item) {
        return;
    }

    item->setParentLayout(this);
    m_items.push_back(std::move(item));
    invalidate();
}

void Layout::insertItem(int index, LayoutItemPtr item)
{
    if (!item) {
        return;
    }

    item->setParentLayout(this);

    const int count = static_cast<int>(m_items.size());
    if (index < 0 || index > count) {
        // 索引超出范围，追加到末尾
        m_items.push_back(std::move(item));
    } else {
        m_items.insert(m_items.begin() + index, std::move(item));
    }
    invalidate();
}

void Layout::removeItem(LayoutItemPtr item)
{
    if (!item) {
        return;
    }

    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        (*it)->setParentLayout(nullptr);
        m_items.erase(it);
        invalidate();
    }
}

void Layout::clear()
{
    // 清除所有子项的父布局引用
    for (auto& item : m_items) {
        if (item) {
            item->setParentLayout(nullptr);
        }
    }
    m_items.clear();
    // 释放 vector 容量，减少内存占用
    m_items.shrink_to_fit();
    invalidate();
}

int Layout::count() const
{
    return static_cast<int>(m_items.size());
}

LayoutItemPtr Layout::itemAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return nullptr;
}

int Layout::indexOf(LayoutItemPtr item) const
{
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        return static_cast<int>(it - m_items.begin());
    }
    return -1;
}

void Layout::setSpacing(int spacing)
{
    if (m_spacing != spacing) {
        m_spacing = spacing;
        invalidate();
    }
}

int Layout::spacing() const
{
    return m_spacing;
}

void Layout::activate()
{
    // 检查几何区域是否有效
    if (!m_geometry.isValid() || m_geometry.isEmpty()) {
        return;
    }

    // 执行布局计算
    if (!m_items.empty()) {
        doLayout(m_geometry);
        m_valid = true;
    }
}

void Layout::update()
{
    invalidate();
    activate();
}

} // namespace VLayout
