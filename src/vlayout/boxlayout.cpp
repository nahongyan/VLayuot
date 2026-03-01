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
    int maxW = horiz ? 0 : SizeMax;
    int maxH = horiz ? SizeMax : 0;
    int minW = 0;
    int minH = 0;
    int hintW = 0;
    int hintH = 0;

    bool hExp = false;
    bool vExp = false;

    int prevNonEmpty = -1;

    for (int i = 0; i < n; ++i) {
        const auto& item = m_items[i];
        LayoutStruct& ls = m_layoutStructs[i];

        const QSize max = item->maximumSize();
        const QSize min = item->minimumSize();
        const QSize hint = item->sizeHint();
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

            maxW += spacing + max.width();
            minW += spacing + min.width();
            hintW += spacing + hint.width();

            if (!ignore) {
                maxH = qMax(maxH, max.height());
                vExp = vExp || (exp & Qt::Vertical);
            }
            minH = qMax(minH, min.height());
            hintH = qMax(hintH, hint.height());

            ls.sizeHint = hint.width();
            ls.maximumSize = max.width();
            ls.minimumSize = min.width();
            ls.expansive = expand;
            ls.stretch = item->stretch();
        } else {
            // 垂直布局
            const bool expand = (exp & Qt::Vertical) || item->stretch() > 0;
            vExp = vExp || expand;

            maxH += spacing + max.height();
            minH += spacing + min.height();
            hintH += spacing + hint.height();

            if (!ignore) {
                maxW = qMax(maxW, max.width());
                hExp = hExp || (exp & Qt::Horizontal);
            }
            minW = qMax(minW, min.width());
            hintW = qMax(hintW, hint.width());

            ls.sizeHint = hint.height();
            ls.maximumSize = max.height();
            ls.minimumSize = min.height();
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
    const QRect available = rect.adjusted(m_leftMargin, m_topMargin, -m_rightMargin, -m_bottomMargin);

    // 直接在 m_layoutStructs 上计算（避免 vector 拷贝）
    const int pos = horiz ? available.x() : available.y();
    const int space = horiz ? available.width() : available.height();

    calculateGeometry(m_layoutStructs, 0, n, pos, space, m_spacing);

    // 应用几何位置到子项
    for (int i = 0; i < n; ++i) {
        const auto& item = m_items[i];
        const LayoutStruct& ls = m_layoutStructs[i];

        QRect itemRect;
        if (horiz) {
            itemRect = QRect(ls.pos, available.y(), ls.size, available.height());
        } else {
            itemRect = QRect(available.x(), ls.pos, available.width(), ls.size);
        }

        // 处理对齐
        const Qt::Alignment align = item->alignment();
        if (align != 0) {
            const QSize hint = item->sizeHint();
            if (horiz && available.height() > hint.height()) {
                itemRect = alignedRect(
                    QRect(itemRect.left(), available.y(), ls.size, available.height()),
                    hint, align);
            } else if (!horiz && available.width() > hint.width()) {
                itemRect = alignedRect(
                    QRect(available.x(), itemRect.top(), available.width(), ls.size),
                    hint, align);
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
