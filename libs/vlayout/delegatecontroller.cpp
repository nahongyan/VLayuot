#include "vlayout/delegatecontroller.h"
#include "vlayout/components.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

namespace VLayout {
// ============================================================================
// DelegateController 实现
// ============================================================================

DelegateController::DelegateController(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

DelegateController::~DelegateController()
{
}

// ============================================================================
// 组件管理
// ============================================================================

DelegateController& DelegateController::addComponent(std::shared_ptr<IComponent> component) const
{
    if (!component) {
        return const_cast<DelegateController&>(*this);
    }

    // 添加到映射表和指针列表
    m_componentMap[component->id()] = component.get();
    m_componentPtrs.push_back(component.get());

    // 添加到所有者列表
    m_components.push_back(std::move(component));

    m_bindingCacheDirty = true;
    return const_cast<DelegateController&>(*this);
}

IComponent* DelegateController::component(const QString& id) const
{
    auto it = m_componentMap.find(id);
    return it != m_componentMap.end() ? it->second : nullptr;
}

void DelegateController::clearComponents() const
{
    m_components.clear();
    m_componentMap.clear();
    m_componentPtrs.clear();
    m_rowItems.clear();
    m_autoBindings.clear();
    m_bindingBuilders.clear();
    m_cachedBindings.clear();
    m_bindingCacheDirty = true;
    m_simpleLayout = false;
    // 重置布局引擎（设置空描述符）
    m_layoutEngine.setDescriptor(LayoutItemDescriptor());
}

// ============================================================================
// 声明式布局
// ============================================================================

void DelegateController::setLayout(const LayoutItemDescriptor& descriptor) const
{
    m_layoutEngine.setDescriptor(descriptor);
    m_simpleLayout = false;
}

bool DelegateController::hasAutoLayout() const
{
    return m_layoutEngine.hasDescriptor();
}

// ============================================================================
// 极简行布局 API
// ============================================================================

void DelegateController::setRow(std::initializer_list<RI> items) const
{
    m_rowItems.assign(items.begin(), items.end());
    m_simpleLayout = true;
    rebuildRowLayout();
}

void DelegateController::addSpacing(int width) const
{
    m_rowItems.push_back(RI("", width));
    m_simpleLayout = true;
    rebuildRowLayout();
}

void DelegateController::addStretch(int stretch) const
{
    m_rowItems.push_back(RI("", -stretch));
    m_simpleLayout = true;
    rebuildRowLayout();
}

void DelegateController::setMargins(int horizontal, int vertical) const
{
    m_marginLeft = m_marginRight = horizontal;
    m_marginTop = m_marginBottom = vertical;
    if (m_simpleLayout) rebuildRowLayout();
}

void DelegateController::setMargins(int left, int top, int right, int bottom) const
{
    m_marginLeft = left;
    m_marginTop = top;
    m_marginRight = right;
    m_marginBottom = bottom;
    if (m_simpleLayout) rebuildRowLayout();
}

void DelegateController::setSpacing(int spacing) const
{
    m_rowSpacing = spacing;
    if (m_simpleLayout) rebuildRowLayout();
}

void DelegateController::setRowHeight(int height) const
{
    // width=1 通过 isValid() 检查，实际宽度由视图控制
    m_fixedSizeHint = QSize(1, height);
}

void DelegateController::rebuildRowLayout() const
{
    LayoutItemList list;
    list.reserve(m_rowItems.size());

    for (const auto& ri : m_rowItems) {
        LayoutItemDescriptor desc;

        if (ri.width > 0) {
            // 固定宽度项
            desc = Item(ri.id, QSize(ri.width, 0));
            // 固定宽项默认垂直居中（除非用户显式指定对齐）
            desc.align(ri.align != Qt::Alignment{} ? ri.align : Qt::AlignVCenter);
        } else if (ri.width == 0) {
            // 宽度由组件 sizeHint 属性决定
            desc = Item(ri.id);
            if (ri.align != Qt::Alignment{}) {
                desc.align(ri.align);
            }
        } else {
            // 弹性填充
            desc = Stretch(ri.id);
            if (ri.align != Qt::Alignment{}) {
                desc.align(ri.align);
            }
        }
        list.push_back(desc);
    }

    LayoutItemDescriptor desc(LayoutItemDescriptor::Kind::HBoxLayout, list);
    desc.margins(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
    desc.spacing(m_rowSpacing);
    m_layoutEngine.setDescriptor(desc);
}

// ============================================================================
// 声明式数据绑定
// ============================================================================

BindingBuilder& DelegateController::bindTo(const QString& componentId) const
{
    auto it = m_bindingBuilders.find(componentId);
    if (it == m_bindingBuilders.end()) {
        auto builder = std::make_unique<BindingBuilder>(componentId);
        auto& ref = *builder;
        m_bindingBuilders[componentId] = std::move(builder);
        m_autoBindings[componentId] = m_bindingBuilders[componentId]->build();
        m_bindingCacheDirty = true;
        return ref;
    }
    return *it->second;
}

// ============================================================================
// 声明式事件处理
// ============================================================================

void DelegateController::onClick(const QString& componentId, ClickHandler handler) const
{
    m_clickHandlers[componentId] = std::move(handler);
}

void DelegateController::onAnyClick(ClickHandler handler) const
{
    m_anyClickHandler = std::move(handler);
}

// ============================================================================
// 便捷方法
// ============================================================================

void DelegateController::setFixedSizeHint(const QSize& size) const
{
    m_fixedSizeHint = size;
}

void DelegateController::toggleData(const QModelIndex& index, int role)
{
    // 验证索引和模型有效性
    if (!index.isValid()) return;
    auto* model = const_cast<QAbstractItemModel*>(index.model());
    if (!model) return;

    bool current = index.data(role).toBool();
    model->setData(index, !current, role);
}

void DelegateController::setModelData(const QModelIndex& index, const QVariant& value, int role)
{
    // 验证索引和模型有效性
    if (!index.isValid()) return;
    auto* model = const_cast<QAbstractItemModel*>(index.model());
    if (!model) return;

    model->setData(index, value, role);
}

// ============================================================================
// paint()
// ============================================================================

void DelegateController::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    m_currentIndex = index;

    // 1. 应用声明式数据绑定
    applyAutoBindings(index);

    // 2. 计算内容区域
    QRect contentRect = option.rect;

    // 3. 执行声明式布局
    if (hasAutoLayout()) {
        applyAutoLayout(contentRect);
    }

    // 4. 初始化共享 ComponentContext
    m_sharedCtx.painter = painter;
    m_sharedCtx.bounds = option.rect;
    m_sharedCtx.index = index;
    m_sharedCtx.option = &option;
    m_sharedCtx.backgroundColor = option.palette.color(QPalette::Window);
    m_sharedCtx.textColor = option.palette.color(QPalette::Text);
    m_sharedCtx.accentColor = option.palette.color(QPalette::Highlight);
    m_sharedCtx.font = option.font;

    const bool isMouseOver = option.state & QStyle::State_MouseOver;
    const bool isSelected = option.state & QStyle::State_Selected;

    // 同步 delegate 级状态到 ComponentContext
    m_sharedCtx.states = int(ComponentState::Enabled);
    if (isMouseOver) m_sharedCtx.states |= int(ComponentState::Hovered);
    if (isSelected) m_sharedCtx.states |= int(ComponentState::Selected);

    // 5. 绘制所有可见组件
    for (auto* comp : m_componentPtrs) {
        if (comp->hasState(ComponentState::Visible)) {
            // 每次 paint 重置交互状态（组件共享于所有行）
            comp->setState(ComponentState::Hovered, isMouseOver);
            comp->setState(ComponentState::Selected, isSelected);
            comp->setState(ComponentState::Pressed, false);

            comp->paint(m_sharedCtx);
        }
    }
}

QSize DelegateController::sizeHint(const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    if (m_fixedSizeHint.isValid()) {
        return m_fixedSizeHint;
    }

    return QSize(100, 30);
}

// ============================================================================
// editorEvent()
// ============================================================================

bool DelegateController::editorEvent(QEvent* event, QAbstractItemModel* model,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index)
{
    Q_UNUSED(model);
    m_currentIndex = index;

    // 同步绑定 + 布局
    applyAutoBindings(index);

    QRect contentRect = option.rect;
    if (hasAutoLayout()) {
        applyAutoLayout(contentRect);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto* me = static_cast<QMouseEvent*>(event);
        m_pressedComponentId.clear();

        if (auto* comp = hitTestComponent(me->pos())) {
            comp->setState(ComponentState::Pressed, true);
            m_pressedComponentId = comp->id();
        }
        break;
    }

    case QEvent::MouseButtonRelease: {
        auto* me = static_cast<QMouseEvent*>(event);

        if (auto* comp = hitTestComponent(me->pos())) {
            comp->setState(ComponentState::Pressed, false);

            // 检查是否为有效点击（按下和释放同一组件）
            if (comp->id() == m_pressedComponentId) {
                comp->onClick(me->pos());
                dispatchClick(index, comp);
                emit componentClicked(index, comp->id());
            }
        }

        m_pressedComponentId.clear();
        break;
    }

    case QEvent::MouseMove: {
        auto* me = static_cast<QMouseEvent*>(event);
        QString newHovered;
        QString newClickableHovered;

        if (auto* comp = hitTestComponent(me->pos())) {
            newHovered = comp->id();
            // 检查是否悬停在可点击组件上
            if (m_clickHandlers.count(comp->id()) > 0 ||
                (m_autoBindings.count(comp->id()) > 0 &&
                 m_autoBindings[comp->id()]->clickCallback())) {
                newClickableHovered = comp->id();
            }
            if (newHovered != m_hoveredComponentId) {
                comp->setState(ComponentState::Hovered, true);
                comp->onMouseEnter();
            }
        }

        // 处理悬停状态变更
        if (!m_hoveredComponentId.isEmpty() && m_hoveredComponentId != newHovered) {
            if (auto* oldComp = component(m_hoveredComponentId)) {
                oldComp->setState(ComponentState::Hovered, false);
                oldComp->onMouseLeave();
            }
        }

        // 处理可点击组件悬停状态变更（用于手型光标）
        if (newClickableHovered != m_hoveredClickableId) {
            if (!m_hoveredClickableId.isEmpty()) {
                emit clickableHoverChanged(m_hoveredClickableId, false);
            }
            if (!newClickableHovered.isEmpty()) {
                emit clickableHoverChanged(newClickableHovered, true);
            }
            m_hoveredClickableId = newClickableHovered;
        }

        m_hoveredComponentId = newHovered;
        break;
    }

    case QEvent::MouseButtonDblClick: {
        auto* me = static_cast<QMouseEvent*>(event);
        if (auto* comp = hitTestComponent(me->pos())) {
            emit componentDoubleClicked(index, comp->id());
        }
        break;
    }

    default:
        break;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// ============================================================================
// 私有方法
// ============================================================================

void DelegateController::applyAutoBindings(const QModelIndex& index) const
{
    // 延迟构建缓存的 {comp*, binding*} 对
    if (m_bindingCacheDirty) {
        m_cachedBindings.clear();
        m_cachedBindings.reserve(m_autoBindings.size());

        for (const auto& pair : m_autoBindings) {
            auto* comp = component(pair.first);
            if (comp && pair.second) {
                m_cachedBindings.push_back({comp, pair.second.get()});
            }
        }
        m_bindingCacheDirty = false;
    }

    // 应用所有绑定
    for (const auto& cb : m_cachedBindings) {
        cb.binding->apply(cb.comp, index);
    }
}

void DelegateController::applyAutoLayout(const QRect& contentRect) const
{
    m_layoutEngine.apply(contentRect, [this](const QString& id) -> IComponent* {
        return component(id);
    });
}

void DelegateController::dispatchClick(const QModelIndex& index, IComponent* comp)
{
    // 1. 绑定级 onClick
    auto bindIt = m_autoBindings.find(comp->id());
    if (bindIt != m_autoBindings.end() && bindIt->second) {
        auto& cb = bindIt->second->clickCallback();
        if (cb) cb(index, comp);
    }

    // 2. 组件级 onClick
    auto handlerIt = m_clickHandlers.find(comp->id());
    if (handlerIt != m_clickHandlers.end() && handlerIt->second) {
        handlerIt->second(index, comp);
    }

    // 3. 全局 onClick
    if (m_anyClickHandler) {
        m_anyClickHandler(index, comp);
    }
}

IComponent* DelegateController::hitTestComponent(const QPoint& point) const
{
    // 逆序遍历缓存的裸指针列表（后绘制的组件在上层）
    for (auto it = m_componentPtrs.rbegin(); it != m_componentPtrs.rend(); ++it) {
        if (*it && (*it)->hitTest(point)) {
            return *it;
        }
    }
    return nullptr;
}

} // namespace VLayout
