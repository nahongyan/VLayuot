#ifndef VLAYOUT_SANDBOX_SCENE_H
#define VLAYOUT_SANDBOX_SCENE_H

/**
 * @file sandbox_scene.h
 * @brief 布局调试场景
 *
 * 管理 QGraphicsView 的场景，包含容器和布局项。
 * 使用真实的 BoxLayout 进行布局计算。
 */

#include "../global.h"
#include "../boxlayout.h"
#include "sandbox_item.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <memory>
#include <vector>

namespace VLayout {

// 前向声明
class ContainerGraphicsItem;
class LayoutItemGraphics;

/**
 * @class SandboxScene
 * @brief 布局调试用的 QGraphicsScene
 *
 * 持有真实的 BoxLayout 实例，确保预览与实际布局行为完全一致。
 */
class VLAYOUT_EXPORT SandboxScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit SandboxScene(QObject* parent = nullptr);
    ~SandboxScene() override;

    // ========== 布局访问（核心接口）==========

    /// 获取底层布局（直接操作）
    std::shared_ptr<BoxLayout> layout() const { return m_layout; }

    /// 设置布局方向
    void setDirection(BoxLayout::Direction direction);

    // ========== 容器设置 ==========

    void setContainerSize(int width, int height);
    QSize containerSize() const { return m_containerSize; }

    void setMargins(int left, int top, int right, int bottom);
    void getMargins(int* left, int* top, int* right, int* bottom) const;

    // ========== 兼容接口（委托给 BoxLayout）==========

    void setSpacing(int spacing);
    int spacing() const;

    BoxLayout::Direction direction() const;

    // ========== 布局项管理（兼容旧接口）==========

    void setItems(const std::vector<SandboxItem>& items);
    const std::vector<SandboxItem>& items() const { return m_items; }

    void clearItems();
    void clearLayout();

    // ========== 选中项 ==========

    void setSelectedIndex(int index);
    int selectedIndex() const { return m_selectedIndex; }

    // ========== 布局计算 ==========

    void computeLayout();

    // ========== 诊断信息 ==========

    QString diagnosticText() const { return m_diagnosticText; }

signals:
    void itemClicked(int index);
    void itemDoubleClicked(int index);
    void layoutComputed(const QString& summary);
    void containerSizeChanged(int width, int height);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void createContainerItem();
    void updateLayoutItems();
    void rebuildLayoutItems();
    QString generateDiagnostics() const;

    // ========== 成员变量 ==========

    std::shared_ptr<BoxLayout> m_layout;  ///< 真实布局实例

    QSize m_containerSize = QSize(400, 200);
    int m_leftMargin = 8;
    int m_topMargin = 8;
    int m_rightMargin = 8;
    int m_bottomMargin = 8;

    std::vector<SandboxItem> m_items;  ///< 显示数据缓存（兼容旧接口）
    int m_selectedIndex = -1;

    QString m_diagnosticText;

    // 图形项
    ContainerGraphicsItem* m_containerItem = nullptr;
    std::vector<LayoutItemGraphics*> m_layoutItems;

    // 网格设置
    int m_gridSmallStep = 10;
    int m_gridLargeStep = 50;
};

} // namespace VLayout

#endif // VLAYOUT_SANDBOX_SCENE_H
