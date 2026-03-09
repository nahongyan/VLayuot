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

#include "flowview.h"
#include "flowlayoutengine.h"
#include "../framework.h"
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>

namespace VLayout {

class FlowView::Private
{
public:
    Private(FlowView* q) : q(q), layoutEngine(new FlowLayoutEngine()) {}

    ~Private() {
        delete layoutEngine;
    }

    FlowView* q;
    QAbstractItemModel* model = nullptr;
    DelegateController* delegate = nullptr;
    FlowLayoutEngine* layoutEngine = nullptr;

    int currentIndex = -1;
    int scrollY = 0;

    // Interaction state
    int hoveredIndex = -1;
    int pressedIndex = -1;
};

FlowView::FlowView(QWidget* parent)
    : QAbstractScrollArea(parent)
    , d(std::make_unique<Private>(this))
{
    setViewport(new QWidget(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        d->scrollY = value;
        viewport()->update();
    });
}

FlowView::~FlowView() = default;

void FlowView::setModel(QAbstractItemModel* model)
{
    if (d->model) {
        disconnect(d->model, nullptr, this, nullptr);
    }

    d->model = model;
    d->layoutEngine->setModel(model);

    if (d->model) {
        connect(d->model, &QAbstractItemModel::modelReset, this, &FlowView::onModelReset);
        connect(d->model, &QAbstractItemModel::rowsInserted, this, &FlowView::onRowsInserted);
        connect(d->model, &QAbstractItemModel::rowsRemoved, this, &FlowView::onRowsRemoved);
        connect(d->model, &QAbstractItemModel::rowsMoved, this, &FlowView::onRowsMoved);
        connect(d->model, &QAbstractItemModel::dataChanged, this, &FlowView::onDataChanged);

        d->layoutEngine->reset(d->model->rowCount());
        d->layoutEngine->setDelegate(d->delegate);
    } else {
        d->layoutEngine->reset(0);
    }

    updateScrollBars();
    emit countChanged(count());
}

QAbstractItemModel* FlowView::model() const
{
    return d->model;
}

void FlowView::setDelegate(DelegateController* delegate)
{
    d->delegate = delegate;
    d->layoutEngine->setDelegate(delegate);
    d->layoutEngine->invalidateAll();
    viewport()->update();
}

DelegateController* FlowView::delegate() const
{
    return d->delegate;
}

void FlowView::scrollTo(int index, ScrollHint hint)
{
    if (!d->model || index < 0 || index >= d->model->rowCount()) return;

    int itemY = d->layoutEngine->itemY(index);
    int h = itemHeight(index);
    int viewportHeight = height();

    int newY = d->scrollY;

    switch (hint) {
    case EnsureVisible:
        if (itemY < d->scrollY) {
            newY = itemY;
        } else if (itemY + h > d->scrollY + viewportHeight) {
            newY = itemY + h - viewportHeight;
        }
        break;
    case PositionAtTop:
        newY = itemY;
        break;
    case PositionAtCenter:
        newY = itemY - (viewportHeight - h) / 2;
        break;
    case PositionAtBottom:
        newY = itemY + h - viewportHeight;
        break;
    }

    verticalScrollBar()->setValue(newY);
}

void FlowView::scrollToTop()
{
    verticalScrollBar()->setValue(0);
}

void FlowView::scrollToBottom()
{
    if (!d->model || d->model->rowCount() == 0) return;
    scrollTo(d->model->rowCount() - 1, PositionAtBottom);
}

int FlowView::currentIndex() const
{
    return d->currentIndex;
}

void FlowView::setCurrentIndex(int index)
{
    if (d->currentIndex == index) return;

    int oldIndex = d->currentIndex;
    d->currentIndex = index;
    emit currentChanged(index, oldIndex);
}

int FlowView::count() const
{
    return d->model ? d->model->rowCount() : 0;
}

QRect FlowView::visualRect(int index) const
{
    if (!d->model || index < 0 || index >= d->model->rowCount()) return QRect();

    int y = d->layoutEngine->itemY(index) - d->scrollY;
    int h = itemHeight(index);

    return QRect(0, y, width(), h);
}

int FlowView::indexAt(const QPoint& point) const
{
    return d->layoutEngine->indexAtY(point.y() + d->scrollY);
}

void FlowView::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e)

    if (!d->model || !d->delegate) {
        QPainter p(viewport());
        p.fillRect(viewport()->rect(), palette().color(QPalette::Base));
        return;
    }

    QPainter painter(viewport());
    painter.setClipRect(viewport()->rect());
    painter.fillRect(viewport()->rect(), palette().color(QPalette::Base));

    // Get visible range
    auto [first, last] = d->layoutEngine->visibleRange(d->scrollY, viewport()->height());

    if (first < 0 || last < 0) return;

    first = qMax(0, first);
    last = qMin(d->model->rowCount() - 1, last);

    // Paint visible items using VLayout delegate
    QStyleOptionViewItem option;
    option.palette = palette();
    option.font = font();
    option.fontMetrics = QFontMetrics(option.font);

    for (int i = first; i <= last; ++i) {
        int itemY = d->layoutEngine->itemY(i) - d->scrollY;
        int h = itemHeight(i);

        QRect itemRect(0, itemY, width(), h);
        option.rect = itemRect;

        // State
        option.state = QStyle::State_Enabled;
        if (i == d->currentIndex) {
            option.state |= QStyle::State_Selected;
        }
        if (i == d->hoveredIndex) {
            option.state |= QStyle::State_MouseOver;
        }

        // Paint using VLayout delegate
        QModelIndex index = d->model->index(i, 0);
        d->delegate->paint(&painter, option, index);
    }
}

void FlowView::resizeEvent(QResizeEvent* e)
{
    QAbstractScrollArea::resizeEvent(e);

    // 更新 viewport 宽度（现在使用 generation 机制，是 O(1) 操作）
    d->layoutEngine->setViewportWidth(width());

    // 更新滚动条（对于大数据集会使用估算高度）
    updateScrollBars();

    // 重绘
    viewport()->update();
}

void FlowView::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)
    viewport()->update();
}

void FlowView::mousePressEvent(QMouseEvent* e)
{
    if (!d->model || !d->delegate) return;

    int index = indexAt(e->pos());
    if (index >= 0) {
        d->pressedIndex = index;
        setCurrentIndex(index);
        emit pressed(index);
    }

    QAbstractScrollArea::mousePressEvent(e);
}

void FlowView::mouseMoveEvent(QMouseEvent* e)
{
    if (!d->model || !d->delegate) return;

    int index = indexAt(e->pos());
    if (index != d->hoveredIndex) {
        d->hoveredIndex = index;
        viewport()->update();
    }

    QAbstractScrollArea::mouseMoveEvent(e);
}

void FlowView::mouseReleaseEvent(QMouseEvent* e)
{
    if (!d->model || !d->delegate) return;

    int index = indexAt(e->pos());
    if (index >= 0 && index == d->pressedIndex) {
        emit clicked(index);
    }

    d->pressedIndex = -1;
    QAbstractScrollArea::mouseReleaseEvent(e);
}

void FlowView::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!d->model || !d->delegate) return;

    int index = indexAt(e->pos());
    if (index >= 0) {
        emit doubleClicked(index);
    }

    QAbstractScrollArea::mouseDoubleClickEvent(e);
}

void FlowView::wheelEvent(QWheelEvent* e)
{
    QAbstractScrollArea::wheelEvent(e);
}

void FlowView::keyPressEvent(QKeyEvent* e)
{
    if (!d->model) {
        QAbstractScrollArea::keyPressEvent(e);
        return;
    }

    switch (e->key()) {
    case Qt::Key_Up:
        if (d->currentIndex > 0) {
            setCurrentIndex(d->currentIndex - 1);
            scrollTo(d->currentIndex, EnsureVisible);
        }
        break;
    case Qt::Key_Down:
        if (d->currentIndex < d->model->rowCount() - 1) {
            setCurrentIndex(d->currentIndex + 1);
            scrollTo(d->currentIndex, EnsureVisible);
        }
        break;
    case Qt::Key_PageUp:
        scrollTo(d->scrollY - viewport()->height(), EnsureVisible);
        break;
    case Qt::Key_PageDown:
        scrollTo(d->scrollY + viewport()->height(), EnsureVisible);
        break;
    case Qt::Key_Home:
        setCurrentIndex(0);
        scrollToTop();
        break;
    case Qt::Key_End:
        if (d->model->rowCount() > 0) {
            setCurrentIndex(d->model->rowCount() - 1);
            scrollToBottom();
        }
        break;
    default:
        QAbstractScrollArea::keyPressEvent(e);
    }
}

void FlowView::onModelReset()
{
    d->layoutEngine->reset(d->model ? d->model->rowCount() : 0);
    d->currentIndex = -1;
    updateScrollBars();
    viewport()->update();
    emit countChanged(count());
}

void FlowView::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    d->layoutEngine->itemsInserted(first, last - first + 1);
    updateScrollBars();
    viewport()->update();
    emit countChanged(count());
}

void FlowView::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    d->layoutEngine->itemsRemoved(first, last - first + 1);
    updateScrollBars();
    viewport()->update();
    emit countChanged(count());
}

void FlowView::onRowsMoved(const QModelIndex& parent, int start, int end,
                           const QModelIndex& destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(destination)
    d->layoutEngine->itemsMoved(start, row, end - start + 1);
    updateScrollBars();
    viewport()->update();
}

void FlowView::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                              const QVector<int>& roles)
{
    Q_UNUSED(roles)
    d->layoutEngine->invalidate(topLeft.row());
    viewport()->update();
}

void FlowView::updateScrollBars()
{
    int total = d->layoutEngine->totalHeight();
    int viewHeight = viewport()->height();

    verticalScrollBar()->setRange(0, qMax(0, total - viewHeight));
    verticalScrollBar()->setPageStep(viewHeight);
    verticalScrollBar()->setSingleStep(20);
}

int FlowView::itemHeight(int index) const
{
    return d->layoutEngine->itemHeight(index);
}

} // namespace VLayout
