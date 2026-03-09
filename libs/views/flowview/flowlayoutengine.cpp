/****************************************************************************
**
** FlowView - High-performance virtualized list component for Qt
** Built on top of VLayout
**
** MIT License
**
** Copyright (c) 2025
**
****************************************************************************/

#include "flowlayoutengine.h"
#include <vlayout/framework.h>
#include <algorithm>
#include <QDebug>

namespace VLayout {

// ============================================================================
// Construction
// ============================================================================

FlowLayoutEngine::FlowLayoutEngine() = default;

FlowLayoutEngine::~FlowLayoutEngine() = default;

// ============================================================================
// Configuration
// ============================================================================

void FlowLayoutEngine::setModel(QAbstractItemModel* model)
{
    m_model = model;
    invalidateAll();
}

void FlowLayoutEngine::setDelegate(DelegateController* delegate)
{
    m_delegate = delegate;
    invalidateAll();
}

void FlowLayoutEngine::setViewportWidth(int width)
{
    if (m_viewportWidth != width) {
        m_viewportWidth = width;
        // 不增加 generation - 保留缓存，在滚动时按需更新
        // 对于高度依赖宽度的 item，会在访问时重新计算
        m_totalHeightValid = false;
    }
}

// ============================================================================
// Data Management
// ============================================================================

void FlowLayoutEngine::reset(int itemCount)
{
    m_itemCount = itemCount;

    // Resize cache
    m_cache.clear();
    m_cache.resize(itemCount);

    // Reset height cache
    m_totalHeightValid = false;
    m_totalHeight = 0;

    // Choose and build skip list
    m_skipInterval = chooseSkipInterval(itemCount);
    m_skipList.clear();

    if (m_skipInterval > 0) {
        buildSkipList();
    }

    qDebug() << "[FlowLayoutEngine] Reset: count=" << itemCount
             << ", skipInterval=" << m_skipInterval
             << ", skipListSize=" << m_skipList.size();
}

void FlowLayoutEngine::invalidate(int from)
{
    if (from < 0 || from >= m_itemCount) return;

    // 快速路径：只使总高度失效
    // 单个缓存项会在访问时通过 generation 检查来验证
    m_totalHeightValid = false;

    // 清除从 from 开始的 skip list 条目
    if (m_skipInterval > 0) {
        int skipIdx = findSkipPointAtIndex(from);
        if (skipIdx >= 0 && skipIdx < static_cast<int>(m_skipList.size())) {
            m_skipList.resize(skipIdx + 1);
        } else {
            m_skipList.clear();
        }
    }
}

void FlowLayoutEngine::invalidateAll()
{
    // 快速路径：只标记总高度失效，不清空单个缓存
    // 缓存会在实际访问时按需验证和更新
    m_totalHeightValid = false;

    // 不清空缓存，不重建 skip list
    // 对于宽度变化的情况，缓存会在滚动时按需更新
}

// ============================================================================
// Layout Queries
// ============================================================================

std::pair<int, int> FlowLayoutEngine::visibleRange(int scrollY, int viewportHeight) const
{
    if (m_itemCount == 0) return {-1, -1};

    int first = findFirstVisible(scrollY);
    int last = findLastVisible(scrollY, viewportHeight);

    return {first, last};
}

int FlowLayoutEngine::itemY(int index) const
{
    if (index < 0 || index >= m_itemCount) return 0;

    ensureHeightCalculated(index);
    return m_cache[index].y;
}

int FlowLayoutEngine::itemHeight(int index) const
{
    if (index < 0 || index >= m_itemCount) return 0;

    ensureHeightCalculated(index);
    return m_cache[index].height;
}

int FlowLayoutEngine::indexAtY(int y) const
{
    if (m_itemCount == 0 || y < 0) return -1;

    // Find starting point using skip list
    int startIdx = 0;
    int skipIdx = findSkipPointAtY(y);

    if (skipIdx >= 0) {
        startIdx = m_skipList[skipIdx].index;
        // Ensure cache is valid from skip point
        ensureRangeValid(startIdx, startIdx + m_skipInterval);
    }

    // Binary search from starting point
    int left = startIdx;
    int right = m_itemCount - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        int itemTop = m_cache[mid].y;
        int itemBottom = itemTop + m_cache[mid].height;

        if (y >= itemTop && y < itemBottom) {
            return mid;
        } else if (y < itemTop) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}

// ============================================================================
// Incremental Updates
// ============================================================================

void FlowLayoutEngine::itemsInserted(int first, int count)
{
    if (count <= 0) return;

    m_itemCount += count;
    m_cache.insert(m_cache.begin() + first, count, CacheEntry{});

    // Invalidate skip list and rebuild
    if (m_skipInterval > 0) {
        m_skipList.clear();
        m_skipInterval = chooseSkipInterval(m_itemCount);
        buildSkipList();
    }

    recalculateFrom(first);
}

void FlowLayoutEngine::itemsRemoved(int first, int count)
{
    if (count <= 0) return;

    m_itemCount -= count;

    if (first < static_cast<int>(m_cache.size())) {
        int removeCount = std::min(count, static_cast<int>(m_cache.size()) - first);
        m_cache.erase(m_cache.begin() + first, m_cache.begin() + first + removeCount);
    }

    // Invalidate skip list and rebuild
    if (m_skipInterval > 0) {
        m_skipList.clear();
        m_skipInterval = chooseSkipInterval(m_itemCount);
        if (m_skipInterval > 0 && m_itemCount > 0) {
            buildSkipList();
        }
    }

    recalculateFrom(first);
}

void FlowLayoutEngine::itemsMoved(int from, int to, int count)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(count)
    // Simplified: just invalidate everything
    invalidateAll();
}

// ============================================================================
// Statistics
// ============================================================================

int FlowLayoutEngine::itemCount() const
{
    return m_itemCount;
}

int FlowLayoutEngine::totalHeight() const
{
    if (m_totalHeightValid) return m_totalHeight;

    if (m_itemCount == 0) {
        m_totalHeight = 0;
        m_totalHeightValid = true;
        return 0;
    }

    // 对于大数据集，使用估算高度而不是遍历所有项
    // 这在 resize 时特别重要，避免 O(n) 操作
    if (m_itemCount > 5000) {
        // 计算前几个可见项的平均高度来估算
        ensureRangeValid(0, qMin(20, m_itemCount - 1));
        int sum = 0;
        int count = 0;
        for (int i = 0; i < qMin(20, m_itemCount); ++i) {
            sum += m_cache[i].height;
            ++count;
        }
        int avgHeight = count > 0 ? sum / count : 50;
        m_totalHeight = avgHeight * m_itemCount;
        // 注意：不设置 valid，让滚动时逐步修正
        return m_totalHeight;
    }

    // 对于小数据集，计算精确高度
    ensureRangeValid(0, m_itemCount - 1);

    m_totalHeight = m_cache[m_itemCount - 1].y + m_cache[m_itemCount - 1].height;
    m_totalHeightValid = true;
    return m_totalHeight;
}

bool FlowLayoutEngine::totalHeightValid() const
{
    return m_totalHeightValid;
}

// ============================================================================
// Internal Helper Methods
// ============================================================================

int FlowLayoutEngine::findFirstVisible(int scrollY) const
{
    if (m_itemCount == 0) return -1;

    // Use skip list to find starting point
    int startIdx = 0;
    int startY = 0;
    int skipIdx = findSkipPointAtY(scrollY);

    if (skipIdx >= 0) {
        startIdx = m_skipList[skipIdx].index;
        startY = m_skipList[skipIdx].y;
    }

    // Estimate: we need to search roughly viewportHeight / avgHeight items
    // Start with a reasonable search range
    int searchRange = m_skipInterval * 3;
    int endIdx = std::min(startIdx + searchRange, m_itemCount - 1);
    ensureRangeValidWithY(startIdx, endIdx, startY);

    // Binary search within the cached range first
    int left = startIdx;
    int right = endIdx;
    int result = 0;

    // First, try to find within the cached range
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int itemBottom = m_cache[mid].y + m_cache[mid].height;

        if (itemBottom > scrollY) {
            result = mid;
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    // If we found a result in the cached range, return it
    // The result should be valid if it's within our search range
    if (result >= 0 && result <= endIdx) {
        return result;
    }

    // Otherwise, expand search (this shouldn't happen often)
    left = endIdx;
    right = m_itemCount - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        // Ensure this item is cached
        ensureHeightCalculated(mid);

        int itemBottom = m_cache[mid].y + m_cache[mid].height;

        if (itemBottom > scrollY) {
            result = mid;
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return result;
}

int FlowLayoutEngine::findLastVisible(int scrollY, int viewportHeight) const
{
    if (m_itemCount == 0) return -1;

    int visibleBottom = scrollY + viewportHeight;

    // Use skip list to find starting point
    int startIdx = 0;
    int startY = 0;
    int skipIdx = findSkipPointAtY(visibleBottom);

    if (skipIdx >= 0) {
        startIdx = m_skipList[skipIdx].index;
        startY = m_skipList[skipIdx].y;
    }

    // Estimate search range
    int searchRange = m_skipInterval * 3;
    int endIdx = std::min(startIdx + searchRange, m_itemCount - 1);
    ensureRangeValidWithY(startIdx, endIdx, startY);

    // Binary search within cached range first
    int left = startIdx;
    int right = endIdx;
    int result = std::min(startIdx, m_itemCount - 1);

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (m_cache[mid].y < visibleBottom) {
            result = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    // If result is in cached range and reasonable, return it
    if (result >= endIdx - m_skipInterval) {
        // Need to search further
        left = endIdx;
        right = m_itemCount - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;

            // Ensure this item is cached
            ensureHeightCalculated(mid);

            if (m_cache[mid].y < visibleBottom) {
                result = mid;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }

    return result;
}

void FlowLayoutEngine::ensureHeightCalculated(int index) const
{
    if (index < 0 || index >= m_itemCount) return;
    if (m_cache[index].heightValid && m_cache[index].generation == m_generation) return;

    // Find a valid starting point - prefer skip list for large datasets
    int startFrom = 0;
    int y = 0;

    if (m_skipInterval > 0) {
        int skipIdx = findSkipPointAtIndex(index);
        if (skipIdx >= 0) {
            startFrom = m_skipList[skipIdx].index;
            y = m_skipList[skipIdx].y;
        }
    }

    // Calculate from startFrom to index
    for (int i = startFrom; i <= index; ++i) {
        if (!m_cache[i].heightValid || m_cache[i].generation != m_generation) {
            if (m_delegate && m_model && m_viewportWidth > 0) {
                QModelIndex modelIndex = m_model->index(i, 0);
                QStyleOptionViewItem option;
                option.rect.setWidth(m_viewportWidth);
                QSize hint = m_delegate->sizeHint(option, modelIndex);
                m_cache[i].height = hint.height() > 0 ? hint.height() : 30;
            } else {
                m_cache[i].height = 30;
            }
            m_cache[i].heightValid = true;
            m_cache[i].generation = m_generation;
        }
        m_cache[i].y = y;
        y += m_cache[i].height;
    }
}

void FlowLayoutEngine::ensureRangeValid(int start, int end) const
{
    if (start < 0) start = 0;
    if (end >= m_itemCount) end = m_itemCount - 1;
    if (start > end) return;

    // Find the starting Y position
    int y = 0;
    int from = start;

    if (start > 0) {
        // Find the last valid entry before start
        // 优先使用同 generation 的缓存，其次使用旧缓存作为估算
        if (m_cache[start - 1].heightValid) {
            y = m_cache[start - 1].y + m_cache[start - 1].height;
            // 如果 generation 不匹配，需要验证这个 Y 位置
            if (m_cache[start - 1].generation != m_generation) {
                // 使用旧缓存作为估算，但标记需要重新计算
                // 从最近的有效 skip point 开始验证
                int skipIdx = findSkipPointAtIndex(start);
                if (skipIdx >= 0) {
                    int skipIndex = m_skipList[skipIdx].index;
                    // 从 skip point 开始重新计算，确保 Y 位置正确
                    y = m_skipList[skipIdx].y;
                    from = skipIndex;
                }
            }
        } else {
            // Need to calculate from an earlier point
            int skipIdx = findSkipPointAtIndex(start);
            if (skipIdx >= 0) {
                int skipIndex = m_skipList[skipIdx].index;
                y = m_skipList[skipIdx].y;
                from = skipIndex;
            } else {
                // No skip point, start from 0
                from = 0;
                y = 0;
            }
        }
    }

    // Calculate heights from 'from' to 'end'
    for (int i = from; i <= end; ++i) {
        // Calculate height
        if (!m_cache[i].heightValid || m_cache[i].generation != m_generation) {
            if (m_delegate && m_model && m_viewportWidth > 0) {
                QModelIndex modelIndex = m_model->index(i, 0);
                QStyleOptionViewItem option;
                option.rect.setWidth(m_viewportWidth);
                QSize hint = m_delegate->sizeHint(option, modelIndex);
                m_cache[i].height = hint.height() > 0 ? hint.height() : 30;
            } else {
                m_cache[i].height = 30;
            }
            m_cache[i].heightValid = true;
            m_cache[i].generation = m_generation;
        }

        m_cache[i].y = y;
        y += m_cache[i].height;
    }
}

void FlowLayoutEngine::ensureRangeValidWithY(int start, int end, int startY) const
{
    if (start < 0) start = 0;
    if (end >= m_itemCount) end = m_itemCount - 1;
    if (start > end) return;

    // Use the provided startY directly - this is the key optimization!
    int y = startY;

    // Calculate heights from 'start' to 'end' using provided startY
    for (int i = start; i <= end; ++i) {
        // Calculate height if not already valid or generation mismatch
        if (!m_cache[i].heightValid || m_cache[i].generation != m_generation) {
            if (m_delegate && m_model && m_viewportWidth > 0) {
                QModelIndex modelIndex = m_model->index(i, 0);
                QStyleOptionViewItem option;
                option.rect.setWidth(m_viewportWidth);
                QSize hint = m_delegate->sizeHint(option, modelIndex);
                m_cache[i].height = hint.height() > 0 ? hint.height() : 30;
            } else {
                m_cache[i].height = 30;
            }
            m_cache[i].heightValid = true;
            m_cache[i].generation = m_generation;
        }

        m_cache[i].y = y;
        y += m_cache[i].height;
    }
}

void FlowLayoutEngine::recalculateFrom(int index)
{
    if (index < 0 || index >= m_itemCount) return;

    // Find the starting Y position
    int y = 0;
    if (index > 0) {
        if (m_cache[index - 1].heightValid) {
            y = m_cache[index - 1].y + m_cache[index - 1].height;
        } else {
            // Can't recalculate, just invalidate
            invalidate(index);
            return;
        }
    }

    // Recalculate Y positions
    for (int i = index; i < m_itemCount; ++i) {
        if (!m_cache[i].heightValid) {
            // Can't continue, stop here
            break;
        }
        m_cache[i].y = y;
        y += m_cache[i].height;
    }

    m_totalHeightValid = false;
}

int FlowLayoutEngine::chooseSkipInterval(int itemCount) const
{
    // Adaptive strategy based on item count
    if (itemCount < 1000) {
        return 0;  // No skip list for small datasets
    } else if (itemCount < 10000) {
        return 64;  // 64 items between checkpoints
    } else if (itemCount < 100000) {
        return 256; // 256 items between checkpoints
    } else {
        return 1024; // 1024 items for very large datasets
    }
}

void FlowLayoutEngine::buildSkipList()
{
    if (m_skipInterval <= 0 || m_itemCount == 0) {
        m_skipList.clear();
        return;
    }

    // Pre-allocate space
    int expectedSize = m_itemCount / m_skipInterval + 1;
    m_skipList.clear();
    m_skipList.reserve(expectedSize);

    // Build skip list by calculating heights at checkpoints
    int y = 0;

    for (int i = 0; i < m_itemCount; ++i) {
        if (i % m_skipInterval == 0) {
            m_skipList.push_back({i, y});
        }

        // Calculate height if not cached or generation mismatch
        if (!m_cache[i].heightValid || m_cache[i].generation != m_generation) {
            if (m_delegate && m_model && m_viewportWidth > 0) {
                QModelIndex modelIndex = m_model->index(i, 0);
                QStyleOptionViewItem option;
                option.rect.setWidth(m_viewportWidth);
                QSize hint = m_delegate->sizeHint(option, modelIndex);
                m_cache[i].height = hint.height() > 0 ? hint.height() : 30;
            } else {
                m_cache[i].height = 30;
            }
            m_cache[i].y = y;
            m_cache[i].heightValid = true;
            m_cache[i].generation = m_generation;
        }

        y += m_cache[i].height;
    }

    // Update total height
    m_totalHeight = y;
    m_totalHeightValid = true;
}

int FlowLayoutEngine::findSkipPointAtY(int y) const
{
    if (m_skipList.empty()) return -1;

    // Binary search for the skip point at or before y
    int left = 0;
    int right = static_cast<int>(m_skipList.size()) - 1;
    int result = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (m_skipList[mid].y <= y) {
            result = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return result;
}

int FlowLayoutEngine::findSkipPointAtIndex(int index) const
{
    if (m_skipList.empty()) return -1;

    // Binary search for the skip point at or before index
    int left = 0;
    int right = static_cast<int>(m_skipList.size()) - 1;
    int result = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (m_skipList[mid].index <= index) {
            result = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return result;
}

} // namespace VLayout
