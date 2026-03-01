#ifndef VLAYOUT_LAYOUTDESCRIPTOR_H
#define VLAYOUT_LAYOUTDESCRIPTOR_H

/**
 * @file layoutdescriptor.h
 * @brief 声明式布局描述系统
 *
 * 提供声明式 API 来描述布局结构。用户在构造函数中声明布局结构，
 * 框架在 paint() 和 editorEvent() 中自动执行布局计算。
 *
 * ## 使用示例
 * \code
 * setLayout(HBox(16, 8, 16, 8, 12, {
 *     Item("cover", {40, 40}),
 *     Stretch("info"),
 *     Item("action", {24, 24}),
 * }));
 * \endcode
 */

#include <QString>
#include <QRect>
#include <QSize>
#include <QMargins>
#include <Qt>
#include <vector>
#include <memory>

#include "component.h"
#include "vlayout/boxlayout.h"
#include "vlayout/widgetitem.h"

namespace VLayout {

// ============================================================================
// 前向声明
// ============================================================================

class IComponent;

// ============================================================================
// LayoutItemDescriptor - 布局项描述符
// ============================================================================

class LayoutItemDescriptor;
using LayoutItemList = std::vector<LayoutItemDescriptor>;

/**
 * @class LayoutItemDescriptor
 * @brief 布局项的声明式描述符
 *
 * LayoutItemDescriptor 用于描述布局结构中的单个项，可以是：
 * - 组件项（ComponentItem）：关联一个组件
 * - 水平布局（HBoxLayout）：包含子项的水平盒
 * - 垂直布局（VBoxLayout）：包含子项的垂直盒
 *
 * ## 使用示例
 * \code
 * // 组件项
 * Item("title", {100, 20})
 *
 * // 弹性填充
 * Stretch("spacer")
 *
 * // 嵌套布局
 * HBox({Item("a"), Item("b")})
 * \endcode
 */
class LayoutItemDescriptor
{
public:
    /**
     * @enum Kind
     * @brief 布局项类型
     */
    enum class Kind {
        ComponentItem,  ///< 组件项
        HBoxLayout,     ///< 水平盒式布局
        VBoxLayout      ///< 垂直盒式布局
    };

    /// 默认构造（创建空的组件项）
    LayoutItemDescriptor()
        : m_kind(Kind::ComponentItem) {}

    /**
     * @brief 构造组件项描述符
     * @param componentId 组件标识符
     * @param sizeHint 首选尺寸（可选）
     */
    explicit LayoutItemDescriptor(const QString& componentId, const QSize& sizeHint = QSize())
        : m_kind(Kind::ComponentItem)
        , m_componentId(componentId)
        , m_sizeHint(sizeHint)
    {}

    /**
     * @brief 构造布局描述符
     * @param kind 布局类型
     * @param children 子项列表
     */
    LayoutItemDescriptor(Kind kind, const LayoutItemList& children)
        : m_kind(kind)
        , m_children(children)
    {}

    // ========== 链式配置方法 ==========

    /// 设置首选尺寸
    LayoutItemDescriptor& size(const QSize& s) { m_sizeHint = s; return *this; }

    /// 设置首选尺寸（宽度和高度）
    LayoutItemDescriptor& size(int w, int h) { m_sizeHint = QSize(w, h); return *this; }

    /// 设置 stretch 因子
    LayoutItemDescriptor& stretch(int s) { m_stretch = s; return *this; }

    /// 设置对齐方式
    LayoutItemDescriptor& align(Qt::Alignment a) { m_alignment = a; return *this; }

    /// 设置间距
    LayoutItemDescriptor& spacing(int s) { m_spacing = s; return *this; }

    /// 设置边距（四边分别指定）
    LayoutItemDescriptor& margins(int left, int top, int right, int bottom) {
        m_margins = QMargins(left, top, right, bottom);
        return *this;
    }

    /// 设置边距（四边统一）
    LayoutItemDescriptor& margins(int all) {
        m_margins = QMargins(all, all, all, all);
        return *this;
    }

    /// 设置子项列表
    LayoutItemDescriptor& items(const LayoutItemList& children) {
        m_children = children;
        return *this;
    }

    // ========== 访问器 ==========

    /// 返回布局项类型
    Kind kind() const { return m_kind; }

    /// 返回组件标识符
    const QString& componentId() const { return m_componentId; }

    /// 返回首选尺寸
    const QSize& sizeHint() const { return m_sizeHint; }

    /// 返回 stretch 因子
    int stretchFactor() const { return m_stretch; }

    /// 返回对齐方式
    Qt::Alignment alignment() const { return m_alignment; }

    /// 返回间距
    int spacingValue() const { return m_spacing; }

    /// 返回边距
    const QMargins& marginsValue() const { return m_margins; }

    /// 返回子项列表
    const LayoutItemList& children() const { return m_children; }

private:
    Kind m_kind = Kind::ComponentItem;
    QString m_componentId;
    QSize m_sizeHint;
    int m_stretch = 0;
    Qt::Alignment m_alignment = {};  // 默认不限制，填满可用空间
    int m_spacing = 0;
    QMargins m_margins;
    LayoutItemList m_children;
};

// ============================================================================
// 便捷工厂函数
// ============================================================================

/**
 * @brief 创建组件项描述符
 * @param componentId 组件标识符
 * @param sizeHint 首选尺寸（可选）
 * @return 布局项描述符
 */
inline LayoutItemDescriptor Item(const QString& componentId, const QSize& sizeHint = QSize()) {
    return LayoutItemDescriptor(componentId, sizeHint);
}

/**
 * @brief 创建弹性填充描述符
 * @param componentId 组件标识符（可选）
 * @return 布局项描述符（stretch=1）
 */
inline LayoutItemDescriptor Stretch(const QString& componentId = QString()) {
    LayoutItemDescriptor desc(componentId);
    desc.stretch(1);
    return desc;
}

/**
 * @brief 创建水平布局描述符（无边距）
 * @param children 子项列表
 * @return 布局项描述符
 */
inline LayoutItemDescriptor HBox(const LayoutItemList& children = {}) {
    return LayoutItemDescriptor(LayoutItemDescriptor::Kind::HBoxLayout, children);
}

/**
 * @brief 创建水平布局描述符（带边距和间距）
 * @param left 左边距
 * @param top 上边距
 * @param right 右边距
 * @param bottom 下边距
 * @param sp 间距
 * @param children 子项列表
 * @return 布局项描述符
 */
inline LayoutItemDescriptor HBox(int left, int top, int right, int bottom, int sp,
                                  const LayoutItemList& children = {}) {
    LayoutItemDescriptor desc(LayoutItemDescriptor::Kind::HBoxLayout, children);
    desc.margins(left, top, right, bottom);
    desc.spacing(sp);
    return desc;
}

/**
 * @brief 创建垂直布局描述符（无边距）
 * @param children 子项列表
 * @return 布局项描述符
 */
inline LayoutItemDescriptor VBox(const LayoutItemList& children = {}) {
    return LayoutItemDescriptor(LayoutItemDescriptor::Kind::VBoxLayout, children);
}

/**
 * @brief 创建垂直布局描述符（带边距和间距）
 * @param left 左边距
 * @param top 上边距
 * @param right 右边距
 * @param bottom 下边距
 * @param sp 间距
 * @param children 子项列表
 * @return 布局项描述符
 */
inline LayoutItemDescriptor VBox(int left, int top, int right, int bottom, int sp,
                                  const LayoutItemList& children = {}) {
    LayoutItemDescriptor desc(LayoutItemDescriptor::Kind::VBoxLayout, children);
    desc.margins(left, top, right, bottom);
    desc.spacing(sp);
    return desc;
}

// ============================================================================
// LayoutEngine - 布局引擎
// ============================================================================

/**
 * @class LayoutEngine
 * @brief 布局引擎，支持布局对象缓存和快速路径优化
 *
 * LayoutEngine 负责将 LayoutItemDescriptor 转换为实际的布局计算，
 * 并提供缓存优化以避免重复计算。
 *
 * ## 缓存优化
 * - 首次布局时构建布局对象缓存
 * - 后续布局时复用缓存对象
 * - 如果尺寸和 sizeHint 未变化，使用快速路径直接平移位置
 */
class LayoutEngine
{
public:
    LayoutEngine() = default;

    /**
     * @brief 设置布局描述符
     * @param root 根布局描述符
     */
    void setDescriptor(const LayoutItemDescriptor& root) {
        m_root = root;
        m_cacheBuilt = false;
        m_fastCacheValid = false;
    }

    /**
     * @brief 检查是否已设置布局
     * @return 如果已设置布局返回 true
     */
    bool hasDescriptor() const {
        return !m_root.children().empty() || !m_root.componentId().isEmpty();
    }

    /**
     * @brief 应用布局到指定区域
     * @param rect 目标矩形区域
     * @param componentGetter 组件获取函数
     */
    void apply(const QRect& rect,
               const std::function<IComponent*(const QString&)>& componentGetter) const
    {
        if (!hasDescriptor()) return;

        // 首次调用时构建布局对象缓存
        if (!m_cacheBuilt) {
            buildCache(componentGetter);
            m_cacheBuilt = true;
        }

        // 检查是否可以使用快速路径
        const QSize contentSize = rect.size();
        const bool sameSize = (contentSize == m_lastContentSize);

        // 收集当前 sizeHints 并检查是否有变化
        bool hintsChanged = false;
        if (sameSize && m_fastCacheValid) {
            hintsChanged = collectAndCheckHints();
        } else {
            collectHints();
            hintsChanged = true;  // 首次或尺寸变化，走慢路径
        }

        // 快速路径：尺寸和 sizeHint 都没变化，直接平移
        if (m_fastCacheValid && sameSize && !hintsChanged) {
            const int ox = rect.x();
            const int oy = rect.y();
            for (auto& fc : m_fastPositions) {
                fc.comp->setGeometry(QRect(
                    fc.relRect.x() + ox,
                    fc.relRect.y() + oy,
                    fc.relRect.width(), fc.relRect.height()));
            }
            return;
        }

        // 慢路径：完整布局计算
        int cacheIdx = 0;
        applyWithCacheImpl(m_root, rect, cacheIdx);

        // 保存结果用于下次快速路径
        m_lastContentSize = contentSize;
        m_lastOrigin = rect.topLeft();
        buildFastCache();
        m_fastCacheValid = true;
    }

private:
    // ========== 私有成员变量 ==========

    LayoutItemDescriptor m_root;

    /// 缓存的布局层级
    struct CachedLevel {
        std::shared_ptr<BoxLayout> layout;
        std::vector<LayoutItemPtr> items;                  // WidgetItem shared_ptrs
        std::vector<const LayoutItemDescriptor*> descs;    // 指向 descriptor
        std::vector<IComponent*> comps;                    // 缓存的组件指针
    };
    mutable std::vector<CachedLevel> m_cache;
    mutable bool m_cacheBuilt = false;

    /// 快速路径缓存（跳过 BoxLayout 计算）
    struct FastPosition {
        IComponent* comp;
        QRect relRect;       // 相对位置（相对于上次 origin）
    };
    mutable std::vector<FastPosition> m_fastPositions;
    mutable std::vector<QSize> m_lastHints;     // 上次使用的 sizeHint 快照
    mutable QSize m_lastContentSize;
    mutable QPoint m_lastOrigin;
    mutable bool m_fastCacheValid = false;

    // ========== 私有方法 ==========

    /**
     * @brief 收集所有组件的当前 sizeHint
     */
    void collectHints() const
    {
        m_lastHints.clear();
        for (const auto& level : m_cache) {
            const size_t n = level.items.size();
            for (size_t i = 0; i < n; ++i) {
                const auto* childDesc = level.descs[i];
                QSize sh = resolveHint(childDesc, level.comps[i]);
                m_lastHints.push_back(sh);
            }
        }
    }

    /**
     * @brief 收集 sizeHints 并检查是否有变化
     * @return 如果有变化返回 true
     */
    bool collectAndCheckHints() const
    {
        bool changed = false;
        size_t idx = 0;
        for (const auto& level : m_cache) {
            const size_t n = level.items.size();
            for (size_t i = 0; i < n; ++i) {
                const auto* childDesc = level.descs[i];
                QSize sh = resolveHint(childDesc, level.comps[i]);
                if (idx < m_lastHints.size()) {
                    if (m_lastHints[idx] != sh) {
                        changed = true;
                        m_lastHints[idx] = sh;
                    }
                } else {
                    changed = true;
                    m_lastHints.push_back(sh);
                }
                ++idx;
            }
        }
        return changed;
    }

    /**
     * @brief 解析 sizeHint（从描述符或组件）
     * @param desc 布局项描述符
     * @param comp 组件指针
     * @return 解析出的 sizeHint
     */
    QSize resolveHint(const LayoutItemDescriptor* desc, IComponent* comp) const
    {
        QSize sh = desc->sizeHint();
        if (!sh.isValid() && comp) {
            // 从组件的 sizeHint 属性获取
            QVariant propSh = comp->property("sizeHint");
            if (propSh.canConvert<QSize>()) {
                sh = propSh.value<QSize>();
            }
            // 如果仍然无效，使用组件的默认 sizeHint
            if (!sh.isValid()) {
                sh = comp->sizeHint();
            }
        }
        // 当高度为 0 时从组件补全（alignedRect 需要实际高度）
        if (comp && sh.isValid() && sh.height() == 0) {
            int ch = comp->sizeHint().height();
            if (ch > 0) sh.setHeight(ch);
        }
        return sh;
    }

    /**
     * @brief 构建快速路径缓存
     */
    void buildFastCache() const
    {
        m_fastPositions.clear();
        for (const auto& level : m_cache) {
            const size_t n = level.comps.size();
            for (size_t i = 0; i < n; ++i) {
                IComponent* comp = level.comps[i];
                if (comp) {
                    // 存相对坐标，快速路径直接加新 origin 即可
                    QRect rel = comp->geometry().translated(-m_lastOrigin.x(), -m_lastOrigin.y());
                    m_fastPositions.push_back({comp, rel});
                }
            }
        }
    }

    /**
     * @brief 递归构建缓存层级
     */
    void buildCacheLevel(const LayoutItemDescriptor& desc,
                         const std::function<IComponent*(const QString&)>& componentGetter) const
    {
        if (desc.kind() == LayoutItemDescriptor::Kind::ComponentItem) {
            return;
        }

        bool isHorizontal = (desc.kind() == LayoutItemDescriptor::Kind::HBoxLayout);
        auto layout = std::make_shared<BoxLayout>(
            isHorizontal ? BoxLayout::Direction::LeftToRight
                         : BoxLayout::Direction::TopToBottom);

        const QMargins& m = desc.marginsValue();
        layout->setContentsMargins(m.left(), m.top(), m.right(), m.bottom());
        layout->setSpacing(desc.spacingValue());

        const auto& children = desc.children();
        const size_t n = children.size();

        CachedLevel level;
        level.layout = layout;
        level.items.reserve(n);
        level.descs.reserve(n);
        level.comps.reserve(n);

        for (size_t ci = 0; ci < n; ++ci) {
            const auto& child = children[ci];
            auto item = std::make_shared<WidgetItem>(
                child.componentId(), WidgetType::Custom);
            item->setStretch(child.stretchFactor());
            item->setAlignment(child.alignment());

            layout->addItem(item);
            level.items.push_back(item);
            level.descs.push_back(&child);

            IComponent* comp = nullptr;
            if (child.kind() == LayoutItemDescriptor::Kind::ComponentItem
                && !child.componentId().isEmpty()) {
                comp = componentGetter(child.componentId());
            }
            level.comps.push_back(comp);

            // 递归处理子布局
            if (child.kind() != LayoutItemDescriptor::Kind::ComponentItem) {
                buildCacheLevel(child, componentGetter);
            }
        }

        m_cache.push_back(std::move(level));
    }

    /**
     * @brief 构建布局缓存
     */
    void buildCache(const std::function<IComponent*(const QString&)>& componentGetter) const
    {
        m_cache.clear();
        m_fastCacheValid = false;
        buildCacheLevel(m_root, componentGetter);
    }

    /**
     * @brief 慢路径：完整布局计算
     */
    void applyWithCacheImpl(const LayoutItemDescriptor& desc,
                            const QRect& rect,
                            int& cacheIdx) const
    {
        if (desc.kind() == LayoutItemDescriptor::Kind::ComponentItem) {
            return;
        }

        if (cacheIdx >= static_cast<int>(m_cache.size())) {
            return;
        }

        auto& level = m_cache[cacheIdx];
        ++cacheIdx;

        level.layout->setGeometry(rect);

        const size_t n = level.items.size();

        // 更新 sizeHint 到 WidgetItem + 标记 dirty
        for (size_t i = 0; i < n; ++i) {
            const auto* childDesc = level.descs[i];
            auto* wi = static_cast<WidgetItem*>(level.items[i].get());
            QSize sh = resolveHint(childDesc, level.comps[i]);
            if (sh.isValid()) {
                wi->setSizeHint(sh);
            }
        }

        level.layout->invalidate();
        level.layout->activate();

        // 赋值 geometry 到组件
        for (size_t i = 0; i < n; ++i) {
            QRect itemRect = level.items[i]->finalRect();
            const auto* childDesc = level.descs[i];

            if (childDesc->kind() == LayoutItemDescriptor::Kind::ComponentItem) {
                IComponent* comp = level.comps[i];
                if (comp) {
                    comp->setGeometry(itemRect);
                }
            } else {
                applyWithCacheImpl(*childDesc, itemRect, cacheIdx);
            }
        }
    }
};

} // namespace VLayout

#endif // VLAYOUT_LAYOUTDESCRIPTOR_H
