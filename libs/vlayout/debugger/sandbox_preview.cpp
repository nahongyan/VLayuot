#include "sandbox_scene.h"
#include "../boxlayout.h"
#include "sandbox_preview.h"

#include <QPainter>
#include <QResizeEvent>

namespace VLayout {

// ============================================================================
// SandboxPreview 实现 (QGraphicsView 版本)
// ============================================================================

SandboxPreview::SandboxPreview(QWidget* parent)
    : QGraphicsView(parent)
{
    // 创建场景
    m_scene = new SandboxScene(this);
    setScene(m_scene);

    // 视图设置
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setDragMode(QGraphicsView::RubberBandDrag);
    setMinimumSize(300, 200);

    // 连接场景信号
    connect(m_scene, &SandboxScene::itemClicked,
            this, &SandboxPreview::itemClicked);
    connect(m_scene, &SandboxScene::itemDoubleClicked,
            this, &SandboxPreview::itemDoubleClicked);
    connect(m_scene, &SandboxScene::layoutComputed,
            this, &SandboxPreview::layoutComputed);
    connect(m_scene, &SandboxScene::containerSizeChanged,
            this, &SandboxPreview::containerSizeChanged);
}

SandboxPreview::~SandboxPreview() = default;

// ========== 布局访问 ==========

std::shared_ptr<BoxLayout> SandboxPreview::layout() const
{
    return m_scene->layout();
}

// ========== 容器设置 ==========

void SandboxPreview::setContainerSize(int width, int height)
{
    m_scene->setContainerSize(width, height);
    updateViewTransform();
}

QSize SandboxPreview::containerSize() const
{
    return m_scene->containerSize();
}

void SandboxPreview::setMargins(int left, int top, int right, int bottom)
{
    m_scene->setMargins(left, top, right, bottom);
}

void SandboxPreview::getMargins(int* left, int* top, int* right, int* bottom) const
{
    m_scene->getMargins(left, top, right, bottom);
}

// ========== 布局设置 ==========

void SandboxPreview::setSpacing(int spacing)
{
    m_scene->setSpacing(spacing);
}

int SandboxPreview::spacing() const
{
    return m_scene->spacing();
}

void SandboxPreview::setDirection(BoxLayout::Direction direction)
{
    m_scene->setDirection(direction);
}

BoxLayout::Direction SandboxPreview::direction() const
{
    return m_scene->direction();
}

// ========== 布局项管理 ==========

void SandboxPreview::setItems(const std::vector<SandboxItem>& items)
{
    m_scene->setItems(items);
}

const std::vector<SandboxItem>& SandboxPreview::items() const
{
    return m_scene->items();
}

void SandboxPreview::clearItems()
{
    m_scene->clearItems();
}

void SandboxPreview::clearLayout()
{
    m_scene->clearLayout();
}

// ========== 选中项 ==========

void SandboxPreview::setSelectedIndex(int index)
{
    m_scene->setSelectedIndex(index);
}

int SandboxPreview::selectedIndex() const
{
    return m_scene->selectedIndex();
}

// ========== 布局计算 ==========

void SandboxPreview::computeLayout()
{
    m_scene->computeLayout();
}

// ========== 诊断信息 ==========

QString SandboxPreview::diagnosticText() const
{
    return m_scene->diagnosticText();
}

// ========== 场景访问 ==========

SandboxScene* SandboxPreview::sandboxScene() const
{
    return m_scene;
}

// ========== 绘制 ==========

void SandboxPreview::drawForeground(QPainter* painter, const QRectF& rect)
{
    QGraphicsView::drawForeground(painter, rect);
    drawRuler(painter, rect);
}

void SandboxPreview::drawRuler(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect);

    painter->save();

    // 获取视图的 viewport 坐标
    QPointF topLeft = mapFromScene(sceneRect().topLeft());
    QPointF bottomRight = mapFromScene(sceneRect().bottomRight());

    // 标尺背景
    QColor rulerBgColor(240, 240, 240);
    QColor rulerTextColor(80, 80, 80);

    // 顶部标尺背景
    painter->fillRect(0, 0, viewport()->width(), m_rulerSize, rulerBgColor);
    // 左侧标尺背景
    painter->fillRect(0, 0, m_rulerSize, viewport()->height(), rulerBgColor);

    painter->setPen(rulerTextColor);
    painter->setFont(QFont("Consolas", 8));

    // 获取场景中的容器位置（假设容器在原点附近）
    QSize container = m_scene->containerSize();
    int startX = m_rulerSize;
    int startY = m_rulerSize;

    // 顶部标尺（水平）
    for (int x = 0; x <= container.width(); x += 50) {
        int screenX = startX + x;
        if (screenX > viewport()->width()) break;

        painter->drawLine(screenX, m_rulerSize - 5, screenX, m_rulerSize);
        painter->drawText(screenX + 2, m_rulerSize - 6, QString::number(x));
    }

    // 左侧标尺（垂直）
    for (int y = 0; y <= container.height(); y += 50) {
        int screenY = startY + y;
        if (screenY > viewport()->height()) break;

        painter->drawLine(m_rulerSize - 5, screenY, m_rulerSize, screenY);
        painter->drawText(2, screenY + 4, QString::number(y));
    }

    painter->restore();
}

void SandboxPreview::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateViewTransform();
}

void SandboxPreview::updateViewTransform()
{
    // 可选：自动缩放以适应视图
    // 目前保持 1:1 比例
    resetTransform();
}

} // namespace VLayout
