#ifndef VLAYOUT_SANDBOX_PREVIEW_H
#define VLAYOUT_SANDBOX_PREVIEW_H

/**
 * @file sandbox_preview.h
 * @brief 布局预览控件 (QGraphicsView 版本)
 *
 * 基于 QGraphicsView/Scene 的布局可视化预览区。
 * 使用真实的 BoxLayout 进行布局计算，确保预览与实际布局行为一致。
 */

#include <vlayout/global.h>
#include <vlayout/boxlayout.h>
#include "sandbox_item.h"
#include <QGraphicsView>
#include <QColor>
#include <memory>
#include <vector>

namespace VLayout {

class SandboxScene;

// ============================================================================
// SandboxPreview - 布局预览控件 (QGraphicsView)
// ============================================================================

/**
 * @class SandboxPreview
 * @brief 基于 QGraphicsView 的布局预览控件
 *
 * 使用 QGraphicsView/Scene 架构提供：
 * - 可拖动的容器
 * - 不可拖动但可点击的布局项
 * - 网格背景
 * - 标尺
 *
 * 通过 layout() 方法获取底层 BoxLayout，可以直接操作布局：
 * @code
 * auto layout = preview->layout();
 * layout->clear();
 * layout->addItem(createLabel("name", "Name:"));
 * layout->addItem(createStretch());
 * layout->addItem(createButton("btn", "OK"));
 * preview->computeLayout();
 * @endcode
 */
class SandboxPreview : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SandboxPreview(QWidget* parent = nullptr);
    ~SandboxPreview() override;

    // ========== 布局访问（核心接口）==========

    /// 获取底层布局（直接操作）
    std::shared_ptr<BoxLayout> layout() const;

    // ========== 容器设置 ==========

    void setContainerSize(int width, int height);
    QSize containerSize() const;

    void setMargins(int left, int top, int right, int bottom);
    void getMargins(int* left, int* top, int* right, int* bottom) const;

    // ========== 布局设置（委托给 BoxLayout）==========

    void setSpacing(int spacing);
    int spacing() const;

    void setDirection(BoxLayout::Direction direction);
    BoxLayout::Direction direction() const;

    // ========== 布局项管理 ==========

    void setItems(const std::vector<SandboxItem>& items);
    const std::vector<SandboxItem>& items() const;

    void clearItems();
    void clearLayout();

    // ========== 选中项 ==========

    void setSelectedIndex(int index);
    int selectedIndex() const;

    // ========== 布局计算 ==========

    void computeLayout();

    // ========== 诊断信息 ==========

    QString diagnosticText() const;

    // ========== 场景访问 ==========

    SandboxScene* sandboxScene() const;

signals:
    void itemClicked(int index);
    void itemDoubleClicked(int index);
    void layoutComputed(const QString& summary);
    void containerSizeChanged(int width, int height);

protected:
    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawRuler(QPainter* painter, const QRectF& rect);
    void updateViewTransform();

    SandboxScene* m_scene = nullptr;
    int m_rulerSize = 30;
};

} // namespace VLayout

#endif // VLAYOUT_SANDBOX_PREVIEW_H
