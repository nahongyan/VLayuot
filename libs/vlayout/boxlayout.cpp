#include "vlayout/boxlayout.h"
#include <algorithm>
#include <cmath>

namespace VLayout {

// ============================================================================
// 定点算术（与 Qt 实现相同）
// ============================================================================

/// 定点数类型，使用 8 位小数精度
using Fixed = qint64;

/// 整数转定点数
static inline Fixed toFixed(int i)
{
    return static_cast<Fixed>(i) * 256;
}

/// 定点数转整数（四舍五入）
static inline int fromFixed(Fixed f)
{
    return static_cast<int>((f % 256 < 128) ? f / 256 : 1 + f / 256);
}

// ============================================================================
// BoxLayout 实现
// ============================================================================

BoxLayout::BoxLayout(Direction direction)
    : Layout()
    , m_direction(direction)
{
}

BoxLayout::~BoxLayout()
{
}

void BoxLayout::setDirection(Direction direction)
{
    if (m_direction != direction) {
        m_direction = direction;
        invalidate();
    }
}

BoxLayout::Direction BoxLayout::direction() const
{
    return m_direction;
}

void BoxLayout::setContentsMargins(int left, int top, int right, int bottom)
{
    if (m_leftMargin != left || m_topMargin != top ||
        m_rightMargin != right || m_bottomMargin != bottom) {
        m_leftMargin = left;
        m_topMargin = top;
        m_rightMargin = right;
        m_bottomMargin = bottom;
        invalidate();
    }
}

void BoxLayout::setContentsMargins(int margins)
{
    setContentsMargins(margins, margins, margins, margins);
}

void BoxLayout::getContentsMargins(int* left, int* top, int* right, int* bottom) const
{
    if (left) *left = m_leftMargin;
    if (top) *top = m_topMargin;
    if (right) *right = m_rightMargin;
    if (bottom) *bottom = m_bottomMargin;
}

// ============================================================================
// 尺寸协商
// ============================================================================

QSize BoxLayout::sizeHint() const
{
    if (m_dirty) {
        setupLayoutData();
    }
    return m_cachedSizeHint;
}

QSize BoxLayout::minimumSize() const
{
    if (m_dirty) {
        setupLayoutData();
    }
    return m_cachedMinSize;
}

QSize BoxLayout::maximumSize() const
{
    if (m_dirty) {
        setupLayoutData();
    }
    return m_cachedMaxSize;
}

Qt::Orientations BoxLayout::expandingDirections() const
{
    if (m_dirty) {
        setupLayoutData();
    }
    return m_cachedExpanding;
}

// ============================================================================
// 布局数据设置
// ============================================================================

void BoxLayout::setupLayoutData() const
{
    if (!m_dirty) {
        return;
    }

    const bool horiz = isHorizontal(m_direction);
    const int n = static_cast<int>(m_items.size());

    // 初始化布局结构
    m_layoutStructs.clear();
    m_layoutStructs.resize(n);

    // 累计值
    // 对于非布局方向的尺寸，初始值为 0，然后在循环中取最大值
    // 对于布局方向的尺寸，初始值为 0，然后在循环中累加
    int maxW = 0;
    int maxH = 0;
    int minW = 0;
    int minH = 0;
    int hintW = 0;
    int hintH = 0;

    bool hExp = false;
    bool vExp = false;

    int prevNonEmpty = -1;
    bool firstNonEmpty = true;

    for (int i = 0; i < n; ++i) {
        const auto& item = m_items[i];
        LayoutStruct& ls = m_layoutStructs[i];

        const QSize itemMax = item->maximumSize();
        const QSize itemMin = item->minimumSize();
        const QSize itemHint = item->sizeHint();
        const Qt::Orientations exp = item->expandingDirections();
        const bool empty = item->isEmpty();

        // 处理间距
        int spacing = 0;
        if (!empty) {
            spacing = (prevNonEmpty >= 0) ? m_spacing : 0;
            if (prevNonEmpty >= 0) {
                m_layoutStructs[prevNonEmpty].spacing = spacing;
            }
            prevNonEmpty = i;
        }

        const bool ignore = empty;

        if (horiz) {
            // 水平布局
            const bool expand = (exp & Qt::Horizontal) || item->stretch() > 0;
            hExp = hExp || expand;

            // 布局方向：累加
            maxW += spacing + itemMax.width();
            minW += spacing + itemMin.width();
            hintW += spacing + itemHint.width();

            if (!ignore) {
                // 非布局方向：取最大值
                if (firstNonEmpty) {
                    maxH = itemMax.height();
                    firstNonEmpty = false;
                } else {
                    maxH = qMax(maxH, itemMax.height());
                }
                vExp = vExp || (exp & Qt::Vertical);
            }
            minH = qMax(minH, itemMin.height());
            hintH = qMax(hintH, itemHint.height());

            ls.sizeHint = itemHint.width();
            ls.maximumSize = itemMax.width();
            ls.minimumSize = itemMin.width();
            ls.expansive = expand;
            ls.stretch = item->stretch();
        } else {
            // 垂直布局
            const bool expand = (exp & Qt::Vertical) || item->stretch() > 0;
            vExp = vExp || expand;

            // 布局方向：累加
            maxH += spacing + itemMax.height();
            minH += spacing + itemMin.height();
            hintH += spacing + itemHint.height();

            if (!ignore) {
                // 非布局方向：取最大值
                if (firstNonEmpty) {
                    maxW = itemMax.width();
                    firstNonEmpty = false;
                } else {
                    maxW = qMax(maxW, itemMax.width());
                }
                hExp = hExp || (exp & Qt::Horizontal);
            }
            minW = qMax(minW, itemMin.width());
            hintW = qMax(hintW, itemHint.width());

            ls.sizeHint = itemHint.height();
            ls.maximumSize = itemMax.height();
            ls.minimumSize = itemMin.height();
            ls.expansive = expand;
            ls.stretch = item->stretch();
        }

        ls.empty = empty;
        ls.spacing = 0;
    }

    // 计算扩展方向
    m_cachedExpanding = {};
    if (hExp) m_cachedExpanding |= Qt::Horizontal;
    if (vExp) m_cachedExpanding |= Qt::Vertical;

    // 计算尺寸
    m_cachedMinSize = QSize(minW, minH);
    m_cachedMaxSize = QSize(maxW, maxH).expandedTo(m_cachedMinSize);
    m_cachedSizeHint = QSize(hintW, hintH).expandedTo(m_cachedMinSize).boundedTo(m_cachedMaxSize);

    // 添加边距
    const QSize margins(m_leftMargin + m_rightMargin, m_topMargin + m_bottomMargin);
    m_cachedMinSize += margins;
    m_cachedMaxSize += margins;
    m_cachedSizeHint += margins;

    m_dirty = false;
}

// ============================================================================
// 核心几何计算（Qt 的 qGeomCalc 算法）
// ============================================================================

void BoxLayout::calculateGeometry(std::vector<LayoutStruct>& chain,
                                   int start, int count,
                                   int pos, int space, int spacer)
{
    int totalHint = 0;
    int totalMin = 0;
    int totalStretch = 0;
    int totalSpacing = 0;
    int expandingCount = 0;

    bool allEmptyNonStretch = true;
    int pendingSpacing = -1;
    int spacerCount = 0;

    // 第一遍：收集子项信息
    for (int i = start; i < start + count; ++i) {
        LayoutStruct* ls = &chain[i];
        ls->done = false;

        totalHint += ls->smartSizeHint();
        totalMin += ls->minimumSize;
        totalStretch += ls->stretch;

        if (!ls->empty) {
            if (pendingSpacing >= 0) {
                totalSpacing += pendingSpacing;
                ++spacerCount;
            }
            pendingSpacing = ls->effectiveSpacer(spacer);
        }

        if (ls->expansive) {
            ++expandingCount;
        }

        allEmptyNonStretch = allEmptyNonStretch && ls->empty && !ls->expansive && ls->stretch <= 0;
    }

    int extraSpace = 0;

    // 场景 1：空间小于最小尺寸
    if (space < totalMin + totalSpacing) {
        const int minSize = totalMin + totalSpacing;

        // 按比例收缩间距
        if (spacer >= 0) {
            spacer = minSize > 0 ? spacer * space / minSize : 0;
            totalSpacing = spacer * spacerCount;
        }

        // 排序最小尺寸
        std::vector<int> minSizes;
        minSizes.reserve(count);
        for (int i = start; i < start + count; ++i) {
            minSizes.push_back(chain[i].minimumSize);
        }
        std::sort(minSizes.begin(), minSizes.end());

        int spaceLeft = space - totalSpacing;
        int sum = 0;
        int idx = 0;
        int spaceUsed = 0;
        int current = 0;

        while (idx < count && spaceUsed < spaceLeft) {
            current = minSizes[idx];
            spaceUsed = sum + current * (count - idx);
            sum += current;
            ++idx;
        }
        --idx;

        const int deficit = spaceUsed - spaceLeft;
        const int items = count - idx;

        const int deficitPerItem = deficit / items;
        const int remainder = deficit % items;
        const int maxVal = current - deficitPerItem;

        int rest = 0;
        for (int i = start; i < start + count; ++i) {
            int maxv = maxVal;
            rest += remainder;
            if (rest >= items) {
                --maxv;
                rest -= items;
            }
            LayoutStruct* ls = &chain[i];
            ls->size = qMin(ls->minimumSize, maxv);
            ls->done = true;
        }
    }
    // 场景 2：空间在最小尺寸和首选尺寸之间
    else if (space < totalHint + totalSpacing) {
        int n = count;
        int spaceLeft = space - totalSpacing;
        int overdraft = totalHint - spaceLeft;

        // 处理 minimum >= hint 的子项
        for (int i = start; i < start + count; ++i) {
            LayoutStruct* ls = &chain[i];
            if (!ls->done && ls->minimumSize >= ls->smartSizeHint()) {
                ls->size = ls->smartSizeHint();
                ls->done = true;
                spaceLeft -= ls->smartSizeHint();
                --n;
            }
        }

        bool finished = (n == 0);
        while (!finished) {
            finished = true;
            Fixed fpOver = toFixed(overdraft);
            Fixed fpW = 0;

            for (int i = start; i < start + count; ++i) {
                LayoutStruct* ls = &chain[i];
                if (ls->done) {
                    continue;
                }

                fpW += fpOver / n;
                const int w = fromFixed(fpW);
                ls->size = ls->smartSizeHint() - w;
                fpW -= toFixed(w);

                if (ls->size < ls->minimumSize) {
                    ls->done = true;
                    ls->size = ls->minimumSize;
                    finished = false;
                    overdraft -= ls->smartSizeHint() - ls->minimumSize;
                    --n;
                    break;
                }
            }
        }
    }
    // 场景 3：有额外空间可用
    else {
        int n = count;
        int spaceLeft = space - totalSpacing;

        // 处理固定尺寸和非扩展子项
        for (int i = start; i < start + count; ++i) {
            LayoutStruct* ls = &chain[i];
            if (!ls->done
                && (ls->maximumSize <= ls->smartSizeHint()
                    || (!allEmptyNonStretch && ls->empty && !ls->expansive && ls->stretch == 0))) {
                ls->size = ls->smartSizeHint();
                ls->done = true;
                spaceLeft -= ls->size;
                totalStretch -= ls->stretch;
                if (ls->expansive) {
                    --expandingCount;
                }
                --n;
            }
        }

        extraSpace = spaceLeft;

        // 迭代分配
        int surplus = 0;
        int deficit = 0;

        do {
            surplus = deficit = 0;
            Fixed fpSpace = toFixed(spaceLeft);
            Fixed fpW = 0;

            for (int i = start; i < start + count; ++i) {
                LayoutStruct* ls = &chain[i];
                if (ls->done) {
                    continue;
                }

                extraSpace = 0;
                if (totalStretch > 0) {
                    fpW += (fpSpace * ls->stretch) / totalStretch;
                } else if (expandingCount > 0) {
                    fpW += (fpSpace * (ls->expansive ? 1 : 0)) / expandingCount;
                } else {
                    fpW += fpSpace / n;
                }

                const int w = fromFixed(fpW);
                ls->size = w;
                fpW -= toFixed(w);

                if (w < ls->smartSizeHint()) {
                    deficit += ls->smartSizeHint() - w;
                } else if (w > ls->maximumSize) {
                    surplus += w - ls->maximumSize;
                }
            }

            // 将空间分配给需要更多的子项
            if (deficit > 0 && surplus <= deficit) {
                for (int i = start; i < start + count; ++i) {
                    LayoutStruct* ls = &chain[i];
                    if (!ls->done && ls->size < ls->smartSizeHint()) {
                        ls->size = ls->smartSizeHint();
                        ls->done = true;
                        spaceLeft -= ls->smartSizeHint();
                        totalStretch -= ls->stretch;
                        if (ls->expansive) {
                            --expandingCount;
                        }
                        --n;
                    }
                }
            }

            // 从过多的子项中回收空间
            if (surplus > 0 && surplus >= deficit) {
                for (int i = start; i < start + count; ++i) {
                    LayoutStruct* ls = &chain[i];
                    if (!ls->done && ls->size > ls->maximumSize) {
                        ls->size = ls->maximumSize;
                        ls->done = true;
                        spaceLeft -= ls->maximumSize;
                        totalStretch -= ls->stretch;
                        if (ls->expansive) {
                            --expandingCount;
                        }
                        --n;
                    }
                }
            }
        } while (n > 0 && surplus != deficit);

        if (n == 0) {
            extraSpace = spaceLeft;
        }
    }

    // 最终位置分配
    const int extra = extraSpace / (spacerCount + 2);
    int p = pos + extra;

    for (int i = start; i < start + count; ++i) {
        LayoutStruct* ls = &chain[i];
        ls->pos = p;
        p += ls->size;
        if (!ls->empty) {
            p += ls->effectiveSpacer(spacer) + extra;
        }
    }
}

// ============================================================================
// 布局执行
// ============================================================================

QRect BoxLayout::alignedRect(const QRect& available, const QSize& size, Qt::Alignment alignment)
{
    int x = available.x();
    int y = available.y();
    int w = qMin(size.width(), available.width());
    int h = qMin(size.height(), available.height());

    // 水平对齐
    if (alignment & Qt::AlignHCenter) {
        x += (available.width() - w) / 2;
    } else if (alignment & Qt::AlignRight) {
        x += available.width() - w;
    }

    // 垂直对齐
    if (alignment & Qt::AlignVCenter) {
        y += (available.height() - h) / 2;
    } else if (alignment & Qt::AlignBottom) {
        y += available.height() - h;
    }

    return QRect(x, y, w, h);
}

void BoxLayout::doLayout(const QRect& rect)
{
    if (m_items.empty()) {
        return;
    }

    setupLayoutData();

    const bool horiz = isHorizontal(m_direction);
    const int n = static_cast<int>(m_items.size());

    // 计算可用区域（减去边距）
    // 当边距之和超过容器尺寸时，可用宽/高 clamp 到 0，等同于 Qt 的行为：
    // 子项按照 space=0 处理，不会因负数传入 qBound 导致 max < min 断言失败。
    QRect available = rect.adjusted(m_leftMargin, m_topMargin, -m_rightMargin, -m_bottomMargin);
    if (available.width() < 0) {
        available.setWidth(0);
    }
    if (available.height() < 0) {
        available.setHeight(0);
    }

    // 直接在 m_layoutStructs 上计算（避免 vector 拷贝）
    const int pos = horiz ? available.x() : available.y();
    const int space = horiz ? available.width() : available.height();

    calculateGeometry(m_layoutStructs, 0, n, pos, space, m_spacing);

    // 应用几何位置到子项
    for (int i = 0; i < n; ++i) {
        const auto& item = m_items[i];
        LayoutStruct& ls = m_layoutStructs[i];  // 使用引用以便修改

        // 获取子项的对齐方式
        const Qt::Alignment align = item->alignment();

        // 对于单个子项且有布局方向对齐的情况，调整分配空间
        // 使其对齐在整个可用空间内生效
        if (n == 1) {
            if (horiz && (align & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignCenter))) {
                // 水平对齐：将分配空间扩展到整个可用宽度
                ls.pos = available.x();
                ls.size = available.width();
            } else if (!horiz && (align & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter | Qt::AlignCenter))) {
                // 垂直对齐：将分配空间扩展到整个可用高度
                ls.pos = available.y();
                ls.size = available.height();
            }
        }

        // 获取子项的尺寸约束
        const QSize hint = item->sizeHint();
        const QSize minSize = item->minimumSize();
        const QSize maxSize = item->maximumSize();

        QRect itemRect;
        if (horiz) {
            // 水平布局
            // 计算布局方向（水平）的尺寸
            // calculateGeometry 已经在可用空间内做了最优分配，直接使用其结果。
            // 若 space < totalMin（场景1），ls.size 可能 < minSize，这是有意行为，
            // 不再用 qBound 重新强制，否则当 ls.size < minSize 时会导致 max < min 断言。
            int itemWidth = ls.size;
            // 即使无对齐，也要尊重 maxSize 上限（Qt 行为）
            if (maxSize.width() < SizeMax) {
                itemWidth = qMin(itemWidth, maxSize.width());
            }

            // 如果设置了水平方向的对齐，改用 sizeHint 宽度，并在合法范围内 clamp
            // 注意：不检查 AlignCenter，因为它包含垂直对齐
            if (align & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter)) {
                itemWidth = hint.width();
                // 保证 max >= min 再调用 qBound
                const int effMax = (maxSize.width() < SizeMax)
                    ? qMax(maxSize.width(), minSize.width())
                    : qMax(itemWidth, minSize.width());
                itemWidth = qBound(minSize.width(), itemWidth, effMax);
            }

            // 计算非布局方向（垂直）的尺寸
            int itemHeight;
            if (align & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter)) {
                // 有垂直对齐：使用 sizeHint 高度，clamp 到 [min, max] 并不超过 available
                itemHeight = hint.height();
                itemHeight = qMax(itemHeight, minSize.height());
                if (maxSize.height() < SizeMax) {
                    itemHeight = qMin(itemHeight, maxSize.height());
                }
                itemHeight = qMin(itemHeight, available.height());
            } else {
                // 无垂直对齐：填满可用高度，但受 maxSize 上限约束
                itemHeight = available.height();
                if (maxSize.height() < SizeMax) {
                    itemHeight = qMin(itemHeight, maxSize.height());
                }
            }

            itemRect = QRect(ls.pos, available.y(), itemWidth, itemHeight);
        } else {
            // 垂直布局
            // 布局方向（垂直）：calculateGeometry 已做最优分配，直接使用
            int itemHeight = ls.size;
            // 即使无对齐，也要尊重 maxSize 上限（Qt 行为）
            if (maxSize.height() < SizeMax) {
                itemHeight = qMin(itemHeight, maxSize.height());
            }

            // 如果设置了垂直方向的对齐，改用 sizeHint 高度，并在合法范围内 clamp
            // 注意：不检查 AlignCenter，因为它包含水平对齐
            if (align & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter)) {
                itemHeight = hint.height();
                const int effMax = (maxSize.height() < SizeMax)
                    ? qMax(maxSize.height(), minSize.height())
                    : qMax(itemHeight, minSize.height());
                itemHeight = qBound(minSize.height(), itemHeight, effMax);
            }

            // 计算非布局方向（水平）的尺寸
            int itemWidth;
            if (align & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter)) {
                // 有水平对齐：使用 sizeHint 宽度，clamp 到 [min, max] 并不超过 available
                itemWidth = hint.width();
                itemWidth = qMax(itemWidth, minSize.width());
                if (maxSize.width() < SizeMax) {
                    itemWidth = qMin(itemWidth, maxSize.width());
                }
                itemWidth = qMin(itemWidth, available.width());
            } else {
                // 无水平对齐：填满可用宽度，但受 maxSize 上限约束
                itemWidth = available.width();
                if (maxSize.width() < SizeMax) {
                    itemWidth = qMin(itemWidth, maxSize.width());
                }
            }

            itemRect = QRect(available.x(), ls.pos, itemWidth, itemHeight);
        }

        // 处理对齐位置
        if (align != 0) {
            if (horiz) {
                // 水平布局
                // 处理垂直对齐（非布局方向）
                if (available.height() > itemRect.height()) {
                    itemRect = alignedRect(
                        QRect(itemRect.left(), available.y(), itemRect.width(), available.height()),
                        itemRect.size(), align & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter | Qt::AlignCenter));
                }
                // 处理水平对齐（布局方向）
                // 对于有水平对齐的子项，在分配的空间内对齐
                Qt::Alignment hAlign = align & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignCenter);
                if (hAlign) {
                    // 计算分配给此子项的空间范围
                    // 从 ls.pos 开始，宽度为 ls.size
                    QRect allocatedSpace(ls.pos, available.y(), ls.size, available.height());
                    if (allocatedSpace.width() > itemRect.width()) {
                        itemRect = alignedRect(allocatedSpace, itemRect.size(), hAlign);
                    }
                }
            } else {
                // 垂直布局
                // 处理水平对齐（非布局方向）
                if (available.width() > itemRect.width()) {
                    itemRect = alignedRect(
                        QRect(available.x(), itemRect.top(), available.width(), itemRect.height()),
                        itemRect.size(), align & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignCenter));
                }
                // 处理垂直对齐（布局方向）
                // 对于有垂直对齐的子项，在分配的空间内对齐
                Qt::Alignment vAlign = align & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter | Qt::AlignCenter);
                if (vAlign) {
                    QRect allocatedSpace(available.x(), ls.pos, available.width(), ls.size);
                    if (allocatedSpace.height() > itemRect.height()) {
                        itemRect = alignedRect(allocatedSpace, itemRect.size(), vAlign);
                    }
                }
            }
        }

        item->setGeometry(itemRect);
        item->setFinalRect(itemRect);

        // 使用 type() 代替 dynamic_pointer_cast（避免 RTTI 开销）
        const ItemType t = item->type();
        if (t == ItemType::HBox || t == ItemType::VBox) {
            static_cast<Layout*>(item.get())->activate();
        }
    }

    m_valid = true;
}

} // namespace VLayout
