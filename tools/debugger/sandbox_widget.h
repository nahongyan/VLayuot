#ifndef VLAYOUT_SANDBOX_WIDGET_H
#define VLAYOUT_SANDBOX_WIDGET_H

/**
 * @file sandbox_widget.h
 * @brief 布局沙盒窗口
 *
 * 提供完整的布局调试窗口，包含参数面板、预览区和计算结果表格。
 */

#include <vlayout/global.h>
#include <vlayout/boxlayout.h>
#include "sandbox_preview.h"

#include <QWidget>
#include <QSharedPointer>
#include <memory>

class QSpinBox;
class QComboBox;
class QTableWidget;
class QLabel;
class QSplitter;

namespace VLayout {

/**
 * @class LayoutSandboxWidget
 * @brief 布局调试沙盒窗口
 *
 * 提供完整的布局调试功能：
 * - 上方：参数面板（容器尺寸、布局参数、布局项列表）
 * - 下方左：预览区（网格背景 + 布局矩形）
 * - 下方右：计算结果表格
 */
class LayoutSandboxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LayoutSandboxWidget(QWidget* parent = nullptr);
    ~LayoutSandboxWidget() override;

    // ========== 容器设置 ==========

    void setContainerSize(int width, int height);
    QSize containerSize() const;

    void setLayoutDirection(BoxLayout::Direction direction);

    // ========== 布局参数 ==========

    void setSpacing(int spacing);
    void setMargins(int left, int top, int right, int bottom);
    void setMargins(int uniform);

    // ========== 布局项管理 ==========

    /// 添加固定尺寸项
    void addFixedItem(const QString& id, int size, int crossSize = 0);

    /// 添加弹性项
    void addStretchItem(const QString& id, int stretch = 1,
                        int sizeHint = 100, int minSize = 0,
                        int crossSizeHint = 0, int crossMinSize = 0);

    /// 添加间隔
    void addSpacing(int spacing);

    /// 添加自定义项
    void addCustomItem(const QString& id,
                       int sizeHint, int minSize, int maxSize,
                       int stretch = 0,
                       int crossSizeHint = 0, int crossMinSize = 0, int crossMaxSize = 1000000);

    /// 清空所有项
    void clearItems();

    /// 删除指定项
    void removeItem(const QString& id);

    // ========== 从现有布局加载 ==========

    /// 从 BoxLayout 加载
    void loadLayout(std::shared_ptr<BoxLayout> layout);

    // ========== 导出 ==========

    /// 导出为 JSON 配置
    QString exportToJson() const;

    /// 从 JSON 加载
    void loadFromJson(const QString& json);

signals:
    /// 布局计算完成时发出
    void layoutComputed(const QString& summary);

private slots:
    void onContainerSizeChanged();
    void onLayoutParamChanged();
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onItemTableCellChanged(int row, int column);
    void onPreviewItemClicked(int index);
    void onLayoutComputed(const QString& summary);

private:
    void setupUi();
    void setupConnections();
    void updatePreview();
    void updateResultTable();
    void addItemToTable(const SandboxItem& item);
    QString itemStatus(const SandboxItem& item) const;

    // ========== UI 组件 ==========

    // 容器设置
    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QComboBox* m_directionCombo = nullptr;

    // 布局设置
    QSpinBox* m_spacingSpin = nullptr;
    QSpinBox* m_leftMarginSpin = nullptr;
    QSpinBox* m_topMarginSpin = nullptr;
    QSpinBox* m_rightMarginSpin = nullptr;
    QSpinBox* m_bottomMarginSpin = nullptr;

    // 布局项表格
    QTableWidget* m_itemTable = nullptr;

    // 预览和结果
    SandboxPreview* m_preview = nullptr;
    QTableWidget* m_resultTable = nullptr;
    QLabel* m_diagnosticLabel = nullptr;

    // 分割器
    QSplitter* m_horizontalSplitter = nullptr;
    QSplitter* m_verticalSplitter = nullptr;

    // 布局项数据
    std::vector<SandboxItem> m_items;
};

} // namespace VLayout

#endif // VLAYOUT_SANDBOX_WIDGET_H
