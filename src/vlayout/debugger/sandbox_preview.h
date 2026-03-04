#ifndef VLAYOUT_SANDBOX_PREVIEW_H
#define VLAYOUT_SANDBOX_PREVIEW_H

/**
 * @file sandbox_preview.h
 * @brief 布局预览控件
 *
 * 提供带网格背景和标尺的布局可视化预览区。
 */

#include "../global.h"
#include <QWidget>
#include <QColor>
#include <vector>

namespace VLayout {

class BoxLayout;

// ============================================================================
// SandboxItem - 沙盒布局项数据
// ============================================================================

/**
 * @struct SandboxItem
 * @brief 沙盒中单个布局项的数据
 */
struct SandboxItem
{
    QString id;             ///< 项标识
    int sizeHint = 0;       ///< 首选尺寸
    int minSize = 0;        ///< 最小尺寸
    int maxSize = 1000000;  ///< 最大尺寸
    int stretch = 0;        ///< 拉伸因子
    bool isSpacing = false; ///< 是否为间隔项

    // 计算结果（由布局算法填充）
    int pos = 0;            ///< 计算后的位置
    int size = 0;           ///< 计算后的尺寸
    bool isCompressed = false;  ///< 是否被压缩
    bool isOverflow = false;    ///< 是否溢出（小于最小尺寸）
};

// ============================================================================
// SandboxPreview - 布局预览控件
// ============================================================================

/**
 * @class SandboxPreview
 * @brief 带网格背景和标尺的布局预览控件
 *
 * 绘制布局的可视化预览，包括：
 * - 网格背景（每 10px 细格，每 50px 粗格）
 * - 顶部和左侧标尺
 * - 彩色布局矩形
 * - 尺寸标注
 */
class VLAYOUT_EXPORT SandboxPreview : public QWidget
{
    Q_OBJECT

public:
    explicit SandboxPreview(QWidget* parent = nullptr);
    ~SandboxPreview() override;

    // ========== 容器设置 ==========

    void setContainerSize(int width, int height);
    QSize containerSize() const { return m_containerSize; }

    void setMargins(int left, int top, int right, int bottom);
    void getMargins(int* left, int* top, int* right, int* bottom) const;

    // ========== 布局设置 ==========

    void setSpacing(int spacing);
    int spacing() const { return m_spacing; }

    void setDirection(BoxLayout::Direction direction);
    BoxLayout::Direction direction() const { return m_direction; }

    // ========== 布局项管理 ==========

    void setItems(const std::vector<SandboxItem>& items);
    const std::vector<SandboxItem>& items() const { return m_items; }

    void clearItems();

    // ========== 选中项 ==========

    void setSelectedIndex(int index);
    int selectedIndex() const { return m_selectedIndex; }

    // ========== 布局计算 ==========

    void computeLayout();

    // ========== 诊断信息 ==========

    QString diagnosticText() const { return m_diagnosticText; }

signals:
    void itemClicked(int index);
    void layoutComputed(const QString& summary);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    // ========== 绘制方法 ==========

    void drawGrid(QPainter& painter, const QRect& contentRect);
    void drawRuler(QPainter& painter, const QRect& contentRect);
    void drawLayoutItems(QPainter& painter, const QRect& contentRect);
    void drawSingleItem(QPainter& painter, const QRect& itemRect,
                        const SandboxItem& item, int index);

    // ========== 辅助方法 ==========

    QColor itemColor(const SandboxItem& item) const;
    QRect mapToPreview(const QRect& layoutRect, const QRect& contentRect) const;
    int itemAtPosition(const QPoint& pos) const;

    // ========== 成员变量 ==========

    QSize m_containerSize = QSize(400, 200);
    int m_leftMargin = 8;
    int m_topMargin = 8;
    int m_rightMargin = 8;
    int m_bottomMargin = 8;
    int m_spacing = 8;
    BoxLayout::Direction m_direction = BoxLayout::Direction::LeftToRight;

    std::vector<SandboxItem> m_items;
    int m_selectedIndex = -1;
    int m_hoverIndex = -1;

    QString m_diagnosticText;

    // 显示比例
    double m_scale = 1.0;
    int m_rulerSize = 30;  // 标尺区域大小
};

} // namespace VLayout

#endif // VLAYOUT_SANDBOX_PREVIEW_H
