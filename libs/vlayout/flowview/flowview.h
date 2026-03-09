#ifndef VLAYOUT_FLOWVIEW_H
#define VLAYOUT_FLOWVIEW_H

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
 * @file flowview.h
 * @brief High-performance virtualized list view component
 *
 * FlowView provides a high-performance, virtualized list view optimized for
 * displaying large datasets (100k+ items) with smooth scrolling.
 *
 * ## Key Features
 * - Virtualized rendering: Only visible items are rendered
 * - O(log n) binary search for finding visible range
 * - Seamless integration with VLayout's DelegateController
 * - Support for variable-height items
 * - Keyboard navigation support
 *
 * ## Usage Example
 * \code
 * #include <vlayout/framework.h>
 * #include <vlayout/flowview/flowview.h>
 *
 * class ChatDelegate : public VLayout::DelegateController {
 * public:
 *     ChatDelegate() {
 *         addItem<VLayout::AvatarComponent>("avatar", 40);
 *         addItem<VLayout::LabelComponent>("message", -1);
 *         setLayout(VLayout::HBox(8, 8, 8, 8, 8, {
 *             Item("avatar", {40, 40}),
 *             Stretch("message"),
 *         }));
 *         bindTo("message").text(MessageRole);
 *     }
 * };
 *
 * auto* view = new VLayout::FlowView(this);
 * view->setModel(myModel);
 * view->setDelegate(new ChatDelegate(view));
 * \endcode
 */

#include <QAbstractScrollArea>
#include <memory>
#include <QStyleOptionViewItem>

#include "../global.h"

namespace VLayout {

class DelegateController;
class FlowLayoutEngine;

/**
 * @class FlowView
 * @brief High-performance virtualized list view
 *
 * FlowView is a scrollable area that efficiently displays large lists
 * by only rendering visible items. It integrates seamlessly with
 * VLayout's DelegateController for declarative item rendering.
 */
class VLAYOUT_EXPORT FlowView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    /**
     * @brief Scroll hint for scrollTo()
     */
    enum ScrollHint {
        EnsureVisible,      ///< Ensure item is visible
        PositionAtTop,      ///< Position item at top of viewport
        PositionAtCenter,   ///< Position item at center of viewport
        PositionAtBottom    ///< Position item at bottom of viewport
    };
    Q_ENUM(ScrollHint)

    /**
     * @brief Constructor
     */
    explicit FlowView(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~FlowView() override;

    // ========== Model/Delegate ==========

    /**
     * @brief Set the data model
     * @param model The model to display
     */
    void setModel(QAbstractItemModel* model);

    /**
     * @brief Get the current model
     */
    QAbstractItemModel* model() const;

    /**
     * @brief Set the delegate controller for rendering
     * @param delegate The delegate controller
     */
    void setDelegate(DelegateController* delegate);

    /**
     * @brief Get the current delegate
     */
    DelegateController* delegate() const;

    // ========== Navigation ==========

    /**
     * @brief Scroll to a specific item
     * @param index Item index
     * @param hint How to position the item
     */
    void scrollTo(int index, ScrollHint hint = EnsureVisible);

    /**
     * @brief Scroll to the top of the list
     */
    void scrollToTop();

    /**
     * @brief Scroll to the bottom of the list
     */
    void scrollToBottom();

    // ========== Selection ==========

    /**
     * @brief Get the current selected index
     */
    int currentIndex() const;

    /**
     * @brief Set the current selected index
     */
    void setCurrentIndex(int index);

    // ========== Item Information ==========

    /**
     * @brief Get total item count
     */
    int count() const;

    /**
     * @brief Get the visual rect for an item
     * @param index Item index
     * @return Rectangle in viewport coordinates
     */
    QRect visualRect(int index) const;

    /**
     * @brief Get the item index at a viewport position
     * @param point Position in viewport coordinates
     * @return Item index, or -1 if no item at position
     */
    int indexAt(const QPoint& point) const;

signals:
    /**
     * @brief Emitted when the current selection changes
     */
    void currentChanged(int current, int previous);

    /**
     * @brief Emitted when an item is clicked
     */
    void clicked(int index);

    /**
     * @brief Emitted when an item is pressed
     */
    void pressed(int index);

    /**
     * @brief Emitted when an item is double-clicked
     */
    void doubleClicked(int index);

    /**
     * @brief Emitted when the item count changes
     */
    void countChanged(int count);

protected:
    // ========== Event Handlers ==========

    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void scrollContentsBy(int dx, int dy) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private slots:
    void onModelReset();
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onRowsRemoved(const QModelIndex& parent, int first, int last);
    void onRowsMoved(const QModelIndex& parent, int start, int end,
                     const QModelIndex& destination, int row);
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                       const QVector<int>& roles);

private:
    void updateScrollBars();
    int itemHeight(int index) const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace VLayout

#endif // VLAYOUT_FLOWVIEW_H
