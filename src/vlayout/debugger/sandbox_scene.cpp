#include "sandbox_scene.h"
#include "container_graphics_item.h"
#include "layout_item_graphics.h"
#include "../boxlayout.h"
#include "../widgetitem.h"
#include "../spaceritem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

namespace VLayout {

// ============================================================================
// SandboxScene 实现
// ============================================================================

SandboxScene::SandboxScene(QObject* parent)
    : QGraphicsScene(parent)
    , m_layout(std::make_shared<HBoxLayout>())  // 默认水平布局
{
    setBackgroundBrush(QColor(245, 245, 245));
    createContainerItem();
}

SandboxScene::~SandboxScene()
{
    // 图形项由 scene 自动管理
}

// ========== 布局访问 ==========

void SandboxScene::setDirection(BoxLayout::Direction direction)
{
    if (m_layout) {
        m_layout->setDirection(direction);
        computeLayout();
    }
}

BoxLayout::Direction SandboxScene::direction() const
{
    return m_layout ? m_layout->direction() : BoxLayout::Direction::LeftToRight;
}

// ========== 容器设置 ==========

void SandboxScene::setContainerSize(int width, int height)
{
    m_containerSize = QSize(width, height);

    if (m_containerItem) {
        m_containerItem->setSize(m_containerSize);
    }

    setSceneRect(-50, -50, width + 100, height + 100);
    computeLayout();

    emit containerSizeChanged(width, height);
}

void SandboxScene::setMargins(int left, int top, int right, int bottom)
{
    m_leftMargin = left;
    m_topMargin = top;
    m_rightMargin = right;
    m_bottomMargin = bottom;

    if (m_containerItem) {
        m_containerItem->setMargins(left, top, right, bottom);
    }

    if (m_layout) {
        m_layout->setContentsMargins(left, top, right, bottom);
    }

    computeLayout();
}

void SandboxScene::getMargins(int* left, int* top, int* right, int* bottom) const
{
    if (left) *left = m_leftMargin;
    if (top) *top = m_topMargin;
    if (right) *right = m_rightMargin;
    if (bottom) *bottom = m_bottomMargin;
}

// ========== 兼容接口 ==========

void SandboxScene::setSpacing(int spacing)
{
    if (m_layout) {
        m_layout->setSpacing(spacing);
        computeLayout();
    }
}

int SandboxScene::spacing() const
{
    return m_layout ? m_layout->spacing() : 0;
}

// ========== 布局项管理 ==========

void SandboxScene::setItems(const std::vector<SandboxItem>& items)
{
    m_items = items;

    // 重建真实布局
    m_layout = std::make_shared<BoxLayout>(m_layout ? m_layout->direction() : BoxLayout::Direction::LeftToRight);
    m_layout->setContentsMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    m_layout->setSpacing(spacing());

    for (const auto& item : items) {
        if (item.isSpacing) {
            auto spacer = std::make_shared<SpacerItem>(item.sizeHint, 0);
            m_layout->addItem(spacer);
        } else {
            auto widget = std::make_shared<WidgetItem>(item.id);
            widget->setSizeHint(QSize(item.sizeHint, item.crossSizeHint));
            widget->setMinimumSize(QSize(item.minSize, item.crossMinSize));
            widget->setMaximumSize(QSize(item.maxSize, item.crossMaxSize));
            widget->setStretch(item.stretch);
            m_layout->addItem(widget);
        }
    }

    rebuildLayoutItems();
    computeLayout();
}

void SandboxScene::clearItems()
{
    m_items.clear();
    m_selectedIndex = -1;
    m_diagnosticText.clear();

    if (m_layout) {
        m_layout->clear();
    }

    // 清除布局项图形
    for (auto* item : m_layoutItems) {
        removeItem(item);
        delete item;
    }
    m_layoutItems.clear();

    update();
}

void SandboxScene::clearLayout()
{
    clearItems();
}

// ========== 选中项 ==========

void SandboxScene::setSelectedIndex(int index)
{
    m_selectedIndex = index;

    // 更新视觉效果
    for (size_t i = 0; i < m_layoutItems.size(); ++i) {
        m_layoutItems[i]->setSelectedVisual(static_cast<int>(i) == index);
    }
}

// ========== 布局计算 ==========

void SandboxScene::computeLayout()
{
    if (!m_layout || m_layout->count() == 0) {
        m_diagnosticText = QApplication::translate("SandboxScene", "无布局项");
        emit layoutComputed(m_diagnosticText);
        return;
    }

    // 设置布局的几何区域
    QRect layoutRect(m_leftMargin, m_topMargin,
                     m_containerSize.width() - m_leftMargin - m_rightMargin,
                     m_containerSize.height() - m_topMargin - m_bottomMargin);
    m_layout->setGeometry(layoutRect);

    // 执行布局计算
    m_layout->activate();

    // 同步图形项
    updateLayoutItems();

    // 生成诊断信息
    m_diagnosticText = generateDiagnostics();

    emit layoutComputed(m_diagnosticText);
}

QString SandboxScene::generateDiagnostics() const
{
    if (!m_layout) return QString();

    QStringList diagnostics;

    bool isHorizontal = (m_layout->direction() == BoxLayout::Direction::LeftToRight);
    int available = isHorizontal
        ? m_containerSize.width() - m_leftMargin - m_rightMargin
        : m_containerSize.height() - m_topMargin - m_bottomMargin;

    diagnostics << QApplication::translate("SandboxScene", "容器可用空间: %1px").arg(available);

    int used = 0, compressed = 0, overflow = 0;

    for (int i = 0; i < m_layout->count(); ++i) {
        auto item = m_layout->itemAt(i);
        if (!item) continue;

        QRect r = item->finalRect();
        QSize hint = item->sizeHint();
        QSize min = item->minimumSize();

        int mainSize = isHorizontal ? r.width() : r.height();
        int hintSize = isHorizontal ? hint.width() : hint.height();
        int minSize = isHorizontal ? min.width() : min.height();

        used += mainSize;
        if (i < m_layout->count() - 1) {
            used += m_layout->spacing();  // 间距
        }

        if (mainSize < hintSize) compressed++;
        if (mainSize < minSize) overflow++;
    }

    int remaining = available - used;

    if (remaining >= 0) {
        diagnostics << QApplication::translate("SandboxScene", "布局空间充足，剩余 %1px").arg(remaining);
    } else {
        diagnostics << QApplication::translate("SandboxScene", "⚠ 空间不足 %1px").arg(-remaining);
    }

    if (compressed > 0) {
        diagnostics << QApplication::translate("SandboxScene", "压缩项: %1").arg(compressed);
    }
    if (overflow > 0) {
        diagnostics << QApplication::translate("SandboxScene", "⚠ 溢出项: %1").arg(overflow);
    }

    return diagnostics.join("\n");
}

// ========== 背景绘制 ==========

void SandboxScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    // 绘制网格
    painter->save();
    painter->setClipRect(rect);

    QPen thinPen(QColor(220, 220, 220), 1, Qt::DotLine);
    QPen thickPen(QColor(180, 180, 180), 1, Qt::DashLine);

    // 只在可见区域绘制
    int left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % m_gridSmallStep);
    int top = static_cast<int>(rect.top()) - (static_cast<int>(rect.top()) % m_gridSmallStep);
    int right = static_cast<int>(rect.right());
    int bottom = static_cast<int>(rect.bottom());

    // 垂直线
    for (int x = left; x <= right; x += m_gridSmallStep) {
        painter->setPen((x % m_gridLargeStep == 0) ? thickPen : thinPen);
        painter->drawLine(x, static_cast<int>(rect.top()), x, static_cast<int>(rect.bottom()));
    }

    // 水平线
    for (int y = top; y <= bottom; y += m_gridSmallStep) {
        painter->setPen((y % m_gridLargeStep == 0) ? thickPen : thinPen);
        painter->drawLine(static_cast<int>(rect.left()), y, static_cast<int>(rect.right()), y);
    }

    painter->restore();
}

// ========== 私有方法 ==========

void SandboxScene::createContainerItem()
{
    m_containerItem = new ContainerGraphicsItem();
    m_containerItem->setSize(m_containerSize);
    m_containerItem->setMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    addItem(m_containerItem);

    setSceneRect(-50, -50, m_containerSize.width() + 100, m_containerSize.height() + 100);
}

void SandboxScene::updateLayoutItems()
{
    if (!m_layout) return;

    // 确保图形项数量与布局项匹配
    while (static_cast<int>(m_layoutItems.size()) < m_layout->count()) {
        auto* graphics = new LayoutItemGraphics(nullptr, static_cast<int>(m_layoutItems.size()), m_containerItem);
        connect(graphics, &LayoutItemGraphics::clicked, this, &SandboxScene::itemClicked);
        connect(graphics, &LayoutItemGraphics::doubleClicked, this, &SandboxScene::itemDoubleClicked);
        m_layoutItems.push_back(graphics);
        addItem(graphics);
    }
    while (static_cast<int>(m_layoutItems.size()) > m_layout->count()) {
        auto* item = m_layoutItems.back();
        removeItem(item);
        delete item;
        m_layoutItems.pop_back();
    }

    bool isHorizontal = (m_layout->direction() == BoxLayout::Direction::LeftToRight);
    int availableCross = isHorizontal
        ? m_containerSize.height() - m_topMargin - m_bottomMargin
        : m_containerSize.width() - m_leftMargin - m_rightMargin;

    // 同步布局项到图形项
    for (int i = 0; i < m_layout->count(); ++i) {
        auto layoutItem = m_layout->itemAt(i);
        if (!layoutItem) continue;

        auto* graphics = m_layoutItems[i];
        graphics->setLayoutItem(layoutItem.get());
        graphics->setItemIndex(i);
        graphics->syncFromLayoutItem(m_layout->direction(), availableCross);

        // 计算矩形位置（使用 finalRect）
        QRect finalRect = layoutItem->finalRect();
        QRectF itemRect(finalRect);

        // 处理交叉方向尺寸（BoxLayout 只计算主方向）
        if (isHorizontal) {
            itemRect.setHeight(graphics->itemData().crossSize);
        } else {
            itemRect.setWidth(graphics->itemData().crossSize);
        }

        graphics->setRect(itemRect);
        graphics->setSelectedVisual(i == m_selectedIndex);

        // 更新 m_items 缓存（兼容旧接口）
        if (static_cast<int>(m_items.size()) <= i) {
            m_items.push_back(graphics->itemData());
        } else {
            m_items[i] = graphics->itemData();
        }
    }
}

void SandboxScene::rebuildLayoutItems()
{
    // 清除旧的图形项
    for (auto* item : m_layoutItems) {
        removeItem(item);
        delete item;
    }
    m_layoutItems.clear();
    m_items.clear();

    if (!m_layout) return;

    // 为每个布局项创建图形项
    for (int i = 0; i < m_layout->count(); ++i) {
        auto layoutItem = m_layout->itemAt(i);
        auto* graphics = new LayoutItemGraphics(layoutItem.get(), i, m_containerItem);
        connect(graphics, &LayoutItemGraphics::clicked, this, &SandboxScene::itemClicked);
        connect(graphics, &LayoutItemGraphics::doubleClicked, this, &SandboxScene::itemDoubleClicked);
        m_layoutItems.push_back(graphics);
        addItem(graphics);
        m_items.push_back(SandboxItem());  // 占位，将在 updateLayoutItems 中填充
    }

    updateLayoutItems();
}

} // namespace VLayout
