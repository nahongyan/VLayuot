#include "sandbox_preview.h"
#include "../boxlayout.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <cmath>

namespace VLayout {

// ============================================================================
// 颜色常量
// ============================================================================

namespace {
    const QColor GridColor(200, 200, 200);
    const QColor GridColorMajor(150, 150, 150);
    const QColor RulerBgColor(240, 240, 240);
    const QColor RulerTextColor(80, 80, 80);

    const QColor FixedColor(76, 175, 80, 120);      // 绿色
    const QColor StretchColor(33, 150, 243, 120);   // 蓝色
    const QColor SpacingColor(158, 158, 158, 120);  // 灰色
    const QColor CustomColor(255, 152, 0, 120);     // 橙色

    const QColor SelectedColor(255, 235, 59, 180);  // 黄色高亮
    const QColor HoverColor(255, 255, 255, 80);     // 悬停效果
    const QColor MarginColor(200, 50, 50, 60);      // Margin 区域颜色
    const QColor SpacingIndicatorColor(100, 100, 200, 80); // Spacing 指示器颜色
}

// ============================================================================
// SandboxPreview 实现
// ============================================================================

SandboxPreview::SandboxPreview(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(300, 200);
}

SandboxPreview::~SandboxPreview() = default;

// ========== 容器设置 ==========

void SandboxPreview::setContainerSize(int width, int height)
{
    m_containerSize = QSize(width, height);
    computeLayout();
    update();
}

void SandboxPreview::setMargins(int left, int top, int right, int bottom)
{
    m_leftMargin = left;
    m_topMargin = top;
    m_rightMargin = right;
    m_bottomMargin = bottom;
    computeLayout();
    update();
}

void SandboxPreview::getMargins(int* left, int* top, int* right, int* bottom) const
{
    if (left) *left = m_leftMargin;
    if (top) *top = m_topMargin;
    if (right) *right = m_rightMargin;
    if (bottom) *bottom = m_bottomMargin;
}

// ========== 布局设置 ==========

void SandboxPreview::setSpacing(int spacing)
{
    m_spacing = spacing;
    computeLayout();
    update();
}

void SandboxPreview::setDirection(BoxLayout::Direction direction)
{
    m_direction = direction;
    computeLayout();
    update();
}

// ========== 布局项管理 ==========

void SandboxPreview::setItems(const std::vector<SandboxItem>& items)
{
    m_items = items;
    computeLayout();
    update();
}

void SandboxPreview::clearItems()
{
    m_items.clear();
    m_selectedIndex = -1;
    m_hoverIndex = -1;
    m_diagnosticText.clear();
    update();
}

// ========== 选中项 ==========

void SandboxPreview::setSelectedIndex(int index)
{
    m_selectedIndex = index;
    update();
}

// ========== 布局计算 ==========

void SandboxPreview::computeLayout()
{
    if (m_items.empty()) {
        m_diagnosticText = tr("无布局项");
        emit layoutComputed(m_diagnosticText);
        return;
    }

    bool isHorizontal = (m_direction == BoxLayout::Direction::LeftToRight);

    // 计算可用空间
    int available = isHorizontal
        ? m_containerSize.width() - m_leftMargin - m_rightMargin
        : m_containerSize.height() - m_topMargin - m_bottomMargin;

    // 构建 LayoutStruct 数组用于计算
    std::vector<LayoutStruct> structs;
    structs.reserve(m_items.size());

    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];

        LayoutStruct ls;
        ls.sizeHint = item.sizeHint;
        ls.minimumSize = item.minSize;
        ls.maximumSize = item.maxSize;
        ls.stretch = item.stretch;
        ls.empty = false;
        ls.expansive = (item.stretch > 0);

        // 添加间距（除了最后一项）
        if (i < m_items.size() - 1) {
            ls.spacing = m_spacing;
        }

        structs.push_back(ls);
    }

    // 计算总 sizeHint 和总 stretch
    int totalHint = 0;
    int totalStretch = 0;
    int totalMinSize = 0;

    for (const auto& ls : structs) {
        totalHint += ls.smartSizeHint() + ls.effectiveSpacer(m_spacing);
        totalMinSize += ls.minimumSize + ls.effectiveSpacer(m_spacing);
        totalStretch += ls.stretch;
    }

    // 执行布局计算（简化版 qGeomCalc）
    int pos = 0;

    // 第一遍：分配 sizeHint
    for (size_t i = 0; i < structs.size(); ++i) {
        structs[i].pos = pos;
        structs[i].size = structs[i].smartSizeHint();
        pos += structs[i].size + structs[i].effectiveSpacer(m_spacing);
    }

    // 计算剩余空间
    int used = pos - (structs.empty() ? 0 : structs.back().effectiveSpacer(m_spacing));
    int remaining = available - used;

    // 第二遍：分配剩余空间给 stretch 项
    if (remaining > 0 && totalStretch > 0) {
        int extraPerStretch = remaining / totalStretch;
        int extraRemainder = remaining % totalStretch;

        for (auto& ls : structs) {
            if (ls.stretch > 0) {
                int extra = extraPerStretch * ls.stretch;
                if (extraRemainder > 0) {
                    extra += 1;
                    extraRemainder--;
                }
                ls.size = qMin(ls.size + extra, ls.maximumSize);
            }
        }
    }
    // 如果空间不足，需要压缩
    else if (remaining < 0) {
        int deficit = -remaining;

        // 按比例压缩
        for (auto& ls : structs) {
            if (ls.stretch > 0 && ls.size > ls.minimumSize) {
                int reduction = qMin(deficit, ls.size - ls.minimumSize);
                ls.size -= reduction;
                deficit -= reduction;
                if (deficit <= 0) break;
            }
        }
    }

    // 将计算结果写回 m_items
    QStringList diagnostics;
    diagnostics << tr("容器可用空间: %1px").arg(available);
    diagnostics << tr("项总需求: %1px | 最小需求: %2px").arg(totalHint).arg(totalMinSize);

    for (size_t i = 0; i < m_items.size(); ++i) {
        m_items[i].pos = structs[i].pos;
        m_items[i].size = structs[i].size;
        m_items[i].isCompressed = (m_items[i].size < m_items[i].sizeHint);
        m_items[i].isOverflow = (m_items[i].size < m_items[i].minSize);
    }

    if (remaining >= 0) {
        diagnostics << tr("布局空间充足，剩余 %1px").arg(remaining);
    } else {
        diagnostics << tr("⚠ 空间不足 %1px").arg(-remaining);
    }

    m_diagnosticText = diagnostics.join("\n");
    emit layoutComputed(m_diagnosticText);
}

// ========== 绘制事件 ==========

void SandboxPreview::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), Qt::white);

    // 计算内容区域（留出标尺空间）
    QRect contentRect(m_rulerSize, m_rulerSize,
                      width() - m_rulerSize, height() - m_rulerSize);

    // 绘制网格
    drawGrid(painter, contentRect);

    // 绘制标尺
    drawRuler(painter, contentRect);

    // 绘制 margin 指示器
    drawGrid(painter, contentRect);

    // 绘制布局项
    drawLayoutItems(painter, contentRect);
}

// ========== 鼠标事件 ==========

void SandboxPreview::mouseMoveEvent(QMouseEvent* event)
{
    int index = itemAtPosition(event->pos());

    if (index != m_hoverIndex) {
        m_hoverIndex = index;
        update();

        if (index >= 0 && index < static_cast<int>(m_items.size())) {
            const auto& item = m_items[index];
            QString tooltip = tr("%1\npos: %2, size: %3\nhint: %4, stretch: %5")
                .arg(item.id)
                .arg(item.pos).arg(item.size)
                .arg(item.sizeHint).arg(item.stretch);
            QToolTip::showText(event->globalPosition().toPoint(), tooltip);
        } else {
            QToolTip::hideText();
        }
    }
}

void SandboxPreview::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int index = itemAtPosition(event->pos());
        if (index >= 0) {
            m_selectedIndex = index;
            emit itemClicked(index);
            update();
        }
    }
}

void SandboxPreview::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

void SandboxPreview::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int index = itemAtPosition(event->pos());
        if (index >= 0) {
            emit itemDoubleClicked(index);
        }
    }
}

void SandboxPreview::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    m_hoverIndex = -1;
    QToolTip::hideText();
    update();
}

void SandboxPreview::contextMenuEvent(const QPoint& pos)
{
    Q_UNUSED(pos);
    // 右键菜单功能暂时禁用
}

// ========== 绘制方法 ==========

void SandboxPreview::drawGrid(QPainter& painter, const QRect& contentRect)
{
    painter.save();
    painter.setClipRect(contentRect);

    // 绘制网格线
    QPen thinPen(GridColor, 1, Qt::DotLine);
    QPen thickPen(GridColorMajor, 1, Qt::DashLine);

    // 垂直线
    for (int x = contentRect.left(); x <= contentRect.right(); x += 10) {
        painter.setPen((x - contentRect.left()) % 50 == 0 ? thickPen : thinPen);
        painter.drawLine(x, contentRect.top(), x, contentRect.bottom());
    }

    // 水平线
    for (int y = contentRect.top(); y <= contentRect.bottom(); y += 10) {
        painter.setPen((y - contentRect.top()) % 50 == 0 ? thickPen : thinPen);
        painter.drawLine(contentRect.left(), y, contentRect.right(), y);
    }

    painter.restore();
}

void SandboxPreview::drawRuler(QPainter& painter, const QRect& contentRect)
{
    painter.save();

    // 绘制标尺背景
    painter.fillRect(0, 0, width(), m_rulerSize, RulerBgColor);
    painter.fillRect(0, 0, m_rulerSize, height(), RulerBgColor);

    painter.setPen(RulerTextColor);
    painter.setFont(QFont("Consolas", 8));

    // 顶部标尺（水平）
    for (int x = 0; x <= m_containerSize.width(); x += 50) {
        int screenX = contentRect.left() + x;
        if (screenX > contentRect.right()) break;

        painter.drawLine(screenX, m_rulerSize - 5, screenX, m_rulerSize);
        painter.drawText(screenX + 2, m_rulerSize - 6, QString::number(x));
    }

    // 左侧标尺（垂直）
    for (int y = 0; y <= m_containerSize.height(); y += 50) {
        int screenY = contentRect.top() + y;
        if (screenY > contentRect.bottom()) break;

        painter.drawLine(m_rulerSize - 5, screenY, m_rulerSize, screenY);
        painter.drawText(2, screenY + 4, QString::number(y));
    }

    painter.restore();
}

void SandboxPreview::drawLayoutItems(QPainter& painter, const QRect& contentRect)
{
    bool isHorizontal = (m_direction == BoxLayout::Direction::LeftToRight);

    // 先绘制 margin 区域
    painter.save();
    painter.setOpacity(0.3);

    // 左 margin
    if (m_leftMargin > 0) {
        QRect leftMarginRect(contentRect.left(), contentRect.top(),
                             m_leftMargin, contentRect.height());
        painter.fillRect(leftMarginRect, MarginColor);
    }

    // 右 margin
    if (m_rightMargin > 0) {
        int left = contentRect.left() + m_leftMargin +
                   (m_containerSize.width() - m_leftMargin - m_rightMargin);
        QRect rightMarginRect(left, contentRect.top(),
                              m_rightMargin, contentRect.height());
        painter.fillRect(rightMarginRect, MarginColor);
    }

    // 上 margin
    if (m_topMargin > 0) {
        QRect topMarginRect(contentRect.left() + m_leftMargin, contentRect.top(),
                            m_containerSize.width() - m_leftMargin - m_rightMargin, m_topMargin);
        painter.fillRect(topMarginRect, MarginColor);
    }

    // 下 margin
    if (m_bottomMargin > 0) {
        int top = contentRect.top() + m_topMargin +
                  (m_containerSize.height() - m_topMargin - m_bottomMargin);
        QRect bottomMarginRect(contentRect.left() + m_leftMargin, top,
                               m_containerSize.width() - m_leftMargin - m_rightMargin, m_bottomMargin);
        painter.fillRect(bottomMarginRect, MarginColor);
    }

    painter.restore();

    // 绘制 spacing 区域
    if (m_spacing > 0 && m_items.size() > 1) {
        painter.save();
        painter.setOpacity(0.3);

        for (size_t i = 0; i < m_items.size() - 1; ++i) {
            const auto& item = m_items[i];

            if (isHorizontal) {
                int spacingStart = contentRect.left() + m_leftMargin + item.pos + item.size;
                QRect spacingRect(spacingStart, contentRect.top() + m_topMargin,
                                  m_spacing, m_containerSize.height() - m_topMargin - m_bottomMargin);
                painter.fillRect(spacingRect, SpacingIndicatorColor);
            } else {
                int spacingStart = contentRect.top() + m_topMargin + item.pos + item.size;
                QRect spacingRect(contentRect.left() + m_leftMargin, spacingStart,
                                  m_containerSize.width() - m_leftMargin - m_rightMargin, m_spacing);
                painter.fillRect(spacingRect, SpacingIndicatorColor);
            }
        }

        painter.restore();
    }

    // 绘制布局项
    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];

        QRect itemRect;
        if (isHorizontal) {
            itemRect.setX(contentRect.left() + m_leftMargin + item.pos);
            itemRect.setY(contentRect.top() + m_topMargin);
            itemRect.setWidth(item.size);
            itemRect.setHeight(m_containerSize.height() - m_topMargin - m_bottomMargin);
        } else {
            itemRect.setX(contentRect.left() + m_leftMargin);
            itemRect.setY(contentRect.top() + m_topMargin + item.pos);
            itemRect.setWidth(m_containerSize.width() - m_leftMargin - m_rightMargin);
            itemRect.setHeight(item.size);
        }

        drawSingleItem(painter, itemRect, item, static_cast<int>(i));
    }
}

void SandboxPreview::drawSingleItem(QPainter& painter, const QRect& itemRect,
                                     const SandboxItem& item, int index)
{
    painter.save();

    QColor fillColor = itemColor(item);

    // 选中高亮
    if (index == m_selectedIndex) {
        painter.fillRect(itemRect, SelectedColor);
    }

    // 填充
    painter.fillRect(itemRect, fillColor);

    // 边框
    if (index == m_hoverIndex) {
        painter.setPen(QPen(Qt::black, 2));
    } else {
        painter.setPen(QPen(Qt::darkGray, 1));
    }
    painter.drawRect(itemRect);

    // 文字标签
    if (!item.isSpacing) {
        painter.setPen(Qt::black);
        QFont font("Microsoft YaHei", 9);
        painter.setFont(font);

        QString label = QString("%1\n%2px").arg(item.id).arg(item.size);
        if (item.stretch > 0) {
            label += QString("\nstretch=%1").arg(item.stretch);
        }

        painter.drawText(itemRect, Qt::AlignCenter, label);
    }

    // 状态标记
    if (item.isOverflow) {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(itemRect.adjusted(1, 1, -1, -1));
    } else if (item.isCompressed) {
        painter.setPen(QPen(QColor(255, 152, 0), 2));
        painter.drawRect(itemRect.adjusted(1, 1, -1, -1));
    }

    painter.restore();
}

QColor SandboxPreview::itemColor(const SandboxItem& item) const
{
    if (item.isSpacing) {
        return SpacingColor;
    }

    // 固定尺寸项
    if (item.minSize == item.maxSize) {
        return FixedColor;
    }

    // 弹性项
    if (item.stretch > 0) {
        return StretchColor;
    }

    return CustomColor;
}

QRect SandboxPreview::mapToPreview(const QRect& layoutRect, const QRect& contentRect) const
{
    return QRect(
        contentRect.left() + layoutRect.left(),
        contentRect.top() + layoutRect.top(),
        layoutRect.width(),
        layoutRect.height()
    );
}

int SandboxPreview::itemAtPosition(const QPoint& pos) const
{
    if (m_items.empty()) return -1;

    QRect contentRect(m_rulerSize, m_rulerSize,
                      width() - m_rulerSize, height() - m_rulerSize);

    if (!contentRect.contains(pos)) return -1;

    bool isHorizontal = (m_direction == BoxLayout::Direction::LeftToRight);

    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& item = m_items[i];

        QRect itemRect;
        if (isHorizontal) {
            itemRect.setX(contentRect.left() + m_leftMargin + item.pos);
            itemRect.setY(contentRect.top() + m_topMargin);
            itemRect.setWidth(item.size);
            itemRect.setHeight(m_containerSize.height() - m_topMargin - m_bottomMargin);
        } else {
            itemRect.setX(contentRect.left() + m_leftMargin);
            itemRect.setY(contentRect.top() + m_topMargin + item.pos);
            itemRect.setWidth(m_containerSize.width() - m_leftMargin - m_rightMargin);
            itemRect.setHeight(item.size);
        }

        if (itemRect.contains(pos)) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace VLayout
