#ifndef VLAYOUT_FLOWLAYOUTENGINE_H
#define VLAYOUT_FLOWLAYOUTENGINE_H

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

/**
 * @file flowlayoutengine.h
 * @brief Layout engine for FlowView with virtualization support
 *
 * FlowLayoutEngine provides efficient layout calculations for large
 * datasets using:
 * - Adaptive skip-list index for O(skip_interval) random access
 * - Lazy height calculation with caching
 * - Binary search for visible range detection
 * - Incremental updates for insertions/removals
 *
 * ## Performance Characteristics
 *
 * | Data Size  | Strategy          | Random Access | Memory Overhead |
 * |------------|-------------------|---------------|-----------------|
 * | < 1K       | Pure binary       | O(log n)      | 0               |
 * | 1K - 100K  | Skip list (64)    | O(64)         | ~n/64 entries   |
 * | 100K - 1M  | Skip list (256)   | O(256)        | ~n/256 entries  |
 * | > 1M       | Skip list (1024)  | O(1024)       | ~n/1024 entries |
 */

#include <vector>
#include <utility>
#include <QtGlobal>  // for quint32

class QAbstractItemModel;

namespace VLayout {

class DelegateController;

/**
 * @class FlowLayoutEngine
 * @brief Layout engine for virtualized list rendering
 *
 * FlowLayoutEngine manages the layout calculations needed for
 * virtualized list rendering. It caches item positions and heights,
 * and uses an adaptive skip-list index to efficiently find visible items
 * even in very large datasets (1M+ items).
 */
class FlowLayoutEngine
{
public:
    FlowLayoutEngine();
    ~FlowLayoutEngine();

    // ========== Configuration ==========

    /**
     * @brief Set the model for index lookups
     * @param model The data model
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Set the delegate for size hints
     * @param delegate The delegate controller
     */
    void setDelegate(DelegateController* delegate);

    /**
     * @brief Set viewport width for layout calculations
     * @param width Viewport width in pixels
     */
    void setViewportWidth(int width);

    // ========== Data Management ==========

    /**
     * @brief Reset the engine with a new item count
     * @param itemCount Total number of items
     *
     * This method automatically chooses the optimal skip interval
     * based on the item count.
     */
    void reset(int itemCount);

    /**
     * @brief Invalidate cached data from a position onwards
     * @param from Starting index to invalidate
     */
    void invalidate(int from);

    /**
     * @brief Invalidate all cached data
     */
    void invalidateAll();

    // ========== Layout Queries ==========

    /**
     * @brief Get the range of visible items
     * @param scrollY Current scroll position
     * @param viewportHeight Viewport height
     * @return Pair of (first, last) visible indices, or (-1, -1) if empty
     *
     * Uses skip-list accelerated binary search for efficient range detection.
     */
    std::pair<int, int> visibleRange(int scrollY, int viewportHeight) const;

    /**
     * @brief Get the Y position of an item
     * @param index Item index
     * @return Y position in pixels
     */
    int itemY(int index) const;

    /**
     * @brief Get the height of an item
     * @param index Item index
     * @return Height in pixels
     */
    int itemHeight(int index) const;

    /**
     * @brief Find the item at a Y position
     * @param y Y position in content coordinates
     * @return Item index, or -1 if not found
     *
     * Uses skip-list accelerated binary search.
     */
    int indexAtY(int y) const;

    // ========== Incremental Updates ==========

    /**
     * @brief Notify that items were inserted
     * @param first First inserted index
     * @param count Number of items inserted
     */
    void itemsInserted(int first, int count);

    /**
     * @brief Notify that items were removed
     * @param first First removed index
     * @param count Number of items removed
     */
    void itemsRemoved(int first, int count);

    /**
     * @brief Notify that items were moved
     * @param from Source index
     * @param to Destination index
     * @param count Number of items moved
     */
    void itemsMoved(int from, int to, int count);

    // ========== Statistics ==========

    /**
     * @brief Get total item count
     */
    int itemCount() const;

    /**
     * @brief Get total content height
     * @return Total height in pixels
     */
    int totalHeight() const;

    /**
     * @brief Check if total height is cached
     */
    bool totalHeightValid() const;

    // ========== Skip List Debug ==========

    /**
     * @brief Get current skip interval (0 = disabled)
     */
    int skipInterval() const { return m_skipInterval; }

    /**
     * @brief Get skip list entry count
     */
    int skipListSize() const { return static_cast<int>(m_skipList.size()); }

    /**
     * @brief Check if skip list is enabled
     */
    bool hasSkipList() const { return m_skipInterval > 0 && !m_skipList.empty(); }

private:
    // ========== Internal Helper Methods ==========

    /**
     * @brief Find first visible item using skip-list acceleration
     */
    int findFirstVisible(int scrollY) const;

    /**
     * @brief Find last visible item using skip-list acceleration
     */
    int findLastVisible(int scrollY, int viewportHeight) const;

    /**
     * @brief Ensure height is calculated for an item
     *
     * Uses skip-list checkpoint to minimize calculations for random access.
     */
    void ensureHeightCalculated(int index) const;

    /**
     * @brief Ensure heights are calculated for a range
     * @param start Start index (inclusive)
     * @param end End index (inclusive)
     */
    void ensureRangeValid(int start, int end) const;

    /**
     * @brief Ensure heights are calculated for a range with known starting Y
     * @param start Start index (inclusive)
     * @param end End index (inclusive)
     * @param startY Known starting Y position
     *
     * This is more efficient than ensureRangeValid() when we know the starting Y
     * from the skip list.
     */
    void ensureRangeValidWithY(int start, int end, int startY) const;

    /**
     * @brief Recalculate Y positions from an index onwards
     */
    void recalculateFrom(int index);

    /**
     * @brief Choose optimal skip interval based on item count
     */
    int chooseSkipInterval(int itemCount) const;

    /**
     * @brief Build skip list synchronously
     */
    void buildSkipList();

    /**
     * @brief Find skip point at or before a Y position
     * @param y Target Y position
     * @return Index in m_skipList, or -1 if before first checkpoint
     */
    int findSkipPointAtY(int y) const;

    /**
     * @brief Find skip point at or before an item index
     * @param index Target item index
     * @return Index in m_skipList, or -1 if before first checkpoint
     */
    int findSkipPointAtIndex(int index) const;

private:
    // ========== Cache Structures ==========

    /**
     * @brief Cache entry for each item
     */
    struct CacheEntry {
        int y = 0;              ///< Y position (cumulative)
        int height = 0;         ///< Item height
        bool heightValid = false; ///< Whether height is calculated
        quint32 generation = 0; ///< Generation when this was calculated
    };

    /**
     * @brief Skip list entry for O(skip_interval) random access
     *
     * Every N items (where N = m_skipInterval), we store a checkpoint
     * with the cumulative Y position. This allows jumping directly to
     * any region without calculating all preceding heights.
     */
    struct SkipPoint {
        int index;  ///< Item index
        int y;      ///< Cumulative Y position at this item
    };

    // ========== External References ==========

    QAbstractItemModel* m_model = nullptr;
    DelegateController* m_delegate = nullptr;

    // ========== Layout State ==========

    int m_viewportWidth = 0;
    int m_itemCount = 0;

    // ========== Cache Data ==========

    mutable std::vector<CacheEntry> m_cache;
    mutable bool m_totalHeightValid = false;
    mutable int m_totalHeight = 0;
    quint64 m_generation = 0;  ///< Incremented when data changes (使用 64 位避免溢出)

    // ========== Skip List Index ==========

    std::vector<SkipPoint> m_skipList;  ///< Sparse index checkpoints
    int m_skipInterval = 0;              ///< Distance between checkpoints (0 = disabled)
};

} // namespace VLayout

#endif // VLAYOUT_FLOWLAYOUTENGINE_H
