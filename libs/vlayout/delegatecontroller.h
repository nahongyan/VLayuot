#ifndef VLAYOUT_DELEGATECONTROLLER_H
#define VLAYOUT_DELEGATECONTROLLER_H

/**
 * @file delegatecontroller.h
 * @brief Delegate 控制器 - 声明式 QStyledItemDelegate 框架
 *
 * DelegateController 提供纯声明式的 QStyledItemDelegate 实现，
 * 在构造函数中通过声明完成所有配置。
 *
 * ## 使用示例
 * \code
 * class MyDelegate : public DelegateController {
 * public:
 *     MyDelegate(QObject* parent = nullptr) : DelegateController(parent) {
 *         // 添加组件
 *         addItem<LabelComponent>("title");    // 弹性填充
 *         addItem<ButtonComponent>("btn", 60); // 固定宽 60px
 *         addSpacing(16);                      // 右侧留白
 *         setRowHeight(48);
 *
 *         // 数据绑定
 *         bindTo("title").text(Qt::DisplayRole).color(QColor(232, 232, 240));
 *
 *         // 事件处理
 *         onClick("btn", [](const QModelIndex& idx, IComponent*) {
 *             qDebug() << "clicked" << idx.data().toString();
 *         });
 *     }
 * };
 * \endcode
 */

#include "vlayout/component.h"
#include "vlayout/binding.h"
#include "vlayout/layoutdescriptor.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <memory>
#include <vector>
#include <unordered_map>

namespace VLayout {

// ============================================================================
// RI - 行项目描述
// ============================================================================

/**
 * @struct RI
 * @brief 行项目描述 - setRow() 使用的极简布局单元
 *
 * 构造方式：
 * - `{"id"}` - 组件 sizeHint 决定宽度
 * - `{"id", 24}` - 固定宽 24px
 * - `{"id", -1}` - 弹性填充剩余空间
 * - `RI()` - 匿名空白占位（无 id）
 */
struct RI {
    QString id;             ///< 组件标识符
    int width = 0;          ///< 宽度：0=sizeHint, >0=固定像素, <0=弹性填充
    Qt::Alignment align;    ///< 对齐方式：空=填满, Qt::AlignVCenter=垂直居中

    RI() = default;
    RI(const char* id_) : id(id_), width(0) {}
    RI(const QString& id_) : id(id_), width(0) {}
    RI(const char* id_, int w) : id(id_), width(w) {}
    RI(const QString& id_, int w) : id(id_), width(w) {}
    RI(const char* id_, int w, Qt::Alignment a) : id(id_), width(w), align(a) {}
    RI(const QString& id_, int w, Qt::Alignment a) : id(id_), width(w), align(a) {}
};

// ============================================================================
// 类型定义
// ============================================================================

/// 点击事件回调类型
using ClickHandler = std::function<void(const QModelIndex& index, IComponent* component)>;

// ============================================================================
// DelegateController - Delegate 控制器
// ============================================================================

/**
 * @class DelegateController
 * @brief 纯声明式 Delegate 实现
 *
 * DelegateController 继承自 QStyledItemDelegate，提供声明式的 API 来：
 * - 管理组件（addComponent, addItem）
 * - 描述布局（setLayout, setRow）
 * - 绑定数据（bindTo）
 * - 处理事件（onClick）
 *
 * ## 组件管理
 * 使用 addComponent() 注册组件，使用 addItem<T>() 同时注册和加入布局。
 *
 * ## 布局描述
 * - 高级 API：setLayout() 支持任意嵌套布局
 * - 极简 API：setRow()/addItem()/addSpacing() 支持简单的行布局
 *
 * ## 数据绑定
 * 使用 bindTo() 声明从 Model 到组件的数据绑定。
 *
 * ## 事件处理
 * 使用 onClick() 声明点击事件处理函数。
 */
class VLAYOUT_EXPORT DelegateController : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DelegateController(QObject* parent = nullptr);
    ~DelegateController() override;

    // ========== 组件管理 ==========

    /**
     * @brief 注册组件
     * @param component 组件共享指针
     * @return 自身引用，支持链式调用
     */
    DelegateController& addComponent(std::shared_ptr<IComponent> component) const;

    /**
     * @brief 获取组件
     * @param id 组件标识符
     * @return 组件指针，如果未找到返回 nullptr
     */
    IComponent* component(const QString& id) const;

    /**
     * @brief 极简添加组件（仅注册，不加入布局）
     * @tparam T 组件类型
     * @param id 组件标识符
     * @return 自身引用
     */
    template<typename T>
    DelegateController& add(const QString& id) {
        return addComponent(std::make_shared<T>(id));
    }

    /**
     * @brief QBoxLayout 风格：注册组件并追加到行布局
     *
     * @tparam T 组件类型
     * @param id 组件标识符
     * @param width 0=组件 sizeHint 决定宽度, >0=固定像素宽, <0=弹性填充
     * @param align 对齐方式
     * @return 自身引用
     *
     * @code
     * setMargins(0, 0);
     * setSpacing(4);
     * addItem<SpacerComponent>("indent");         // 弹性
     * addItem<ExpandArrowComponent>("arrow", 16); // 固定 16px
     * addItem<LabelComponent>("name");            // 弹性
     * addSpacing(8);                              // 匿名右侧留白
     * setRowHeight(36);
     * @endcode
     */
    template<typename T>
    DelegateController& addItem(const QString& id, int width = 0,
                                Qt::Alignment align = {}) const {
        addComponent(std::make_shared<T>(id));
        m_rowItems.push_back(RI(id, width, align));
        m_simpleLayout = true;
        rebuildRowLayout();
        return const_cast<DelegateController&>(*this);
    }

    /**
     * @brief 获取类型化组件
     * @tparam T 组件类型
     * @param id 组件标识符
     * @return 类型化组件指针，如果未找到或类型不匹配返回 nullptr
     */
    template<typename T>
    T* typedComponent(const QString& id) const {
        auto* comp = component(id);
        return comp ? dynamic_cast<T*>(comp) : nullptr;
    }

    /// 返回内部组件裸指针列表（无堆分配）
    const std::vector<IComponent*>& components() const { return m_componentPtrs; }

    /**
     * @brief 清除所有组件和布局
     *
     * 用于动态切换布局时重置 delegate 状态。
     */
    void clearComponents() const;

    // ========== 声明式布局 ==========

    /**
     * @brief 高级 API：设置任意嵌套布局
     * @param descriptor 布局描述符
     */
    void setLayout(const LayoutItemDescriptor& descriptor) const;

    /// 检查是否已设置自动布局
    bool hasAutoLayout() const;

    // ========== 极简行布局 API ==========

    /**
     * @brief 一次性声明全部行项目（与 addItem 二选一）
     *
     * @code
     * setRow({{"arrow",16}, {"name"}, {"badge",18}, {"",8}});
     * @endcode
     */
    void setRow(std::initializer_list<RI> items) const;

    /**
     * @brief 追加匿名间距（无组件占位，等同 QBoxLayout::addSpacing）
     * @param width 间距宽度（像素）
     */
    void addSpacing(int width) const;

    /**
     * @brief 追加弹簧（等同 QBoxLayout::addStretch，占满剩余空间）
     * @param stretch stretch 因子
     */
    void addStretch(int stretch = 1) const;

    /**
     * @brief 设置行内边距（水平/垂直各一值）
     * @param horizontal 水平边距
     * @param vertical 垂直边距
     */
    void setMargins(int horizontal, int vertical) const;

    /**
     * @brief 设置行内边距（四边分别指定）
     */
    void setMargins(int left, int top, int right, int bottom) const;

    /**
     * @brief 设置组件间间距
     * @param spacing 间距像素值
     */
    void setSpacing(int spacing) const;

    /**
     * @brief 设置固定行高（列表/树视图用）
     * @param height 行高像素值
     */
    void setRowHeight(int height) const;

    // ========== 声明式数据绑定 ==========

    /**
     * @brief 开始组件绑定
     * @param componentId 组件标识符
     * @return 绑定构建器引用
     */
    BindingBuilder& bindTo(const QString& componentId) const;

    // ========== 声明式事件处理 ==========

    /**
     * @brief 设置组件点击事件处理
     * @param componentId 组件标识符
     * @param handler 事件处理函数
     */
    void onClick(const QString& componentId, ClickHandler handler) const;

    /**
     * @brief 设置任意组件点击事件处理
     * @param handler 事件处理函数
     */
    void onAnyClick(ClickHandler handler) const;

    // ========== 便捷方法 ==========

    /// 设置固定 sizeHint（同时设置宽高）
    void setFixedSizeHint(const QSize& size) const;

    /// 切换布尔类型数据
    static void toggleData(const QModelIndex& index, int role);

    /// 设置模型数据
    static void setModelData(const QModelIndex& index, const QVariant& value, int role);

    // ========== QStyledItemDelegate 重写 ==========

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

signals:
    /// 组件点击信号
    void componentClicked(const QModelIndex& index, const QString& componentId);

    /// 组件双击信号
    void componentDoubleClicked(const QModelIndex& index, const QString& componentId);

    /// 组件状态变更信号
    void componentStateChanged(const QModelIndex& index, const QString& componentId,
                               ComponentState state, bool on);

    /// 可点击组件悬停状态变化信号（用于设置手型光标）
    void clickableHoverChanged(const QString& componentId, bool hovered);

private:
    /// 分发点击事件
    void dispatchClick(const QModelIndex& index, IComponent* comp);

protected:
    /// 应用自动绑定
    void applyAutoBindings(const QModelIndex& index) const;

    /// 应用自动布局
    void applyAutoLayout(const QRect& contentRect) const;

    /// 点击测试组件
    IComponent* hitTestComponent(const QPoint& point) const;

private:
    // ========== 组件存储 ==========

    mutable std::vector<std::shared_ptr<IComponent>> m_components;      // 拥有组件
    mutable std::unordered_map<QString, IComponent*> m_componentMap;    // 快速查找
    mutable std::vector<IComponent*> m_componentPtrs;                   // 缓存裸指针

    // ========== 布局 ==========

    mutable LayoutEngine m_layoutEngine;

    // ========== 绑定 ==========

    mutable std::unordered_map<QString, std::unique_ptr<BindingBuilder>> m_bindingBuilders;
    mutable std::unordered_map<QString, std::shared_ptr<ComponentBinding>> m_autoBindings;

    /// 缓存的绑定对 {comp*, binding*}
    struct CachedBinding {
        IComponent* comp;
        ComponentBinding* binding;
    };
    mutable std::vector<CachedBinding> m_cachedBindings;
    mutable bool m_bindingCacheDirty = true;

    // ========== 简单行布局 ==========

    mutable std::vector<RI> m_rowItems;
    mutable int m_marginLeft = 0;
    mutable int m_marginTop = 0;
    mutable int m_marginRight = 0;
    mutable int m_marginBottom = 0;
    mutable int m_rowSpacing = 4;
    mutable bool m_simpleLayout = false;

    void rebuildRowLayout() const;

    // ========== 事件 ==========

    mutable std::unordered_map<QString, ClickHandler> m_clickHandlers;
    mutable ClickHandler m_anyClickHandler;

    // ========== sizeHint ==========

    mutable QSize m_fixedSizeHint;

    // ========== 交互状态 ==========

    mutable QModelIndex m_currentIndex;
    QString m_hoveredComponentId;
    QString m_hoveredClickableId;
    QString m_pressedComponentId;

    // ========== 复用上下文 ==========

    mutable ComponentContext m_sharedCtx;
};

} // namespace VLayout

#endif // VLAYOUT_DELEGATECONTROLLER_H
