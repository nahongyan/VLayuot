#ifndef TIMELINE_COMPONENTS_H
#define TIMELINE_COMPONENTS_H

/**
 * @file timeline_components.h
 * @brief 时间轴自定义组件定义
 *
 * 定义了用于 AI 编程助手时间轴的自定义绘制组件。
 */

#include "vlayout/framework.h"
#include "timeline_roles.h"
#include "markdown_renderer.h"

#include <QColor>
#include <QVector>

namespace Timeline {

/**
 * @class TimelineIndicatorComponent
 * @brief 时间线指示器组件
 *
 * 绘制左侧时间线轨道和圆点标记。
 *
 * ## 布局
 * │
 * ●  ← 圆点 (6px)
 * │
 *
 * ## 功能
 * - 竖线轨道 (1px)
 * - 圆点指示器，颜色根据操作类型变化
 */
class TimelineIndicatorComponent : public VLayout::AbstractComponent
{
public:
    explicit TimelineIndicatorComponent(const QString& id);

    QString type() const override { return QStringLiteral("TimelineIndicator"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

    /// 设置是否为第一个节点（不绘制上半部分竖线）
    void setIsFirst(bool first) { m_isFirst = first; }

    /// 设置是否为最后一个节点（不绘制下半部分竖线）
    void setIsLast(bool last) { m_isLast = last; }

    /// 设置圆点颜色
    void setDotColor(const QColor& color) { m_dotColor = color; }

private:
    bool m_isFirst = false;
    bool m_isLast = false;
    QColor m_dotColor;
};

/**
 * @class MessageContentComponent
 * @brief 消息内容组件 (扁平化设计)
 *
 * 绘制纯文本消息内容，无气泡背景。
 *
 * ## 功能
 * - 纯文本显示，自动换行
 * - 支持流式输出指示器
 * - 扁平化设计，无背景
 * - Markdown 渲染支持
 * - 代码块复制按钮
 */
class MessageContentComponent : public VLayout::AbstractComponent
{
public:
    explicit MessageContentComponent(const QString& id);

    QString type() const override { return QStringLiteral("MessageContent"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

    /**
     * @brief 检测点击位置是否在复制按钮上
     * @param pos 相对于组件的坐标
     * @return 如果在复制按钮上，返回对应的代码块信息；否则返回 nullptr
     */
    const MarkdownRenderer::CodeBlockInfo* hitTestCopyButton(const QPoint& pos) const;

private:
    QSize m_sizeHint;
    QVector<MarkdownRenderer::CodeBlockInfo> m_codeBlocks;  ///< 渲染时的代码块信息
};

/**
 * @class CodeBlockComponent
 * @brief 代码块组件
 *
 * 绘制带语法高亮的代码块。
 *
 * ## 布局
 * ┌─────────────────────────────────┐
 * │ language        [copy] [apply] │
 * ├─────────────────────────────────┤
 * │ code content...               │
 * │ ...                           │
 * └─────────────────────────────────┘
 *
 * ## 功能
 * - 语言标签显示
 * - 复制按钮（带状态反馈）
 * - 应用代码按钮
 * - 基础语法高亮
 */
class CodeBlockComponent : public VLayout::AbstractComponent
{
public:
    explicit CodeBlockComponent(const QString& id);

    QString type() const override { return QStringLiteral("CodeBlock"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;
};

/**
 * @class ToolCardComponent
 * @brief 工具调用卡片组件
 *
 * 绘制工具调用的可视化卡片。
 *
 * ## 布局
 * ┌─────────────────────────────────┐
 * │ [icon] toolName    [status] [▼]│
 * ├─────────────────────────────────┤
 * │ Args: {...}                   │ (可折叠)
 * │ Result: {...}                 │
 * └─────────────────────────────────┘
 *
 * ## 功能
 * - 工具图标 + 名称
 * - 执行状态指示器
 * - 可折叠的参数/结果面板
 */
class ToolCardComponent : public VLayout::AbstractComponent
{
public:
    explicit ToolCardComponent(const QString& id);

    QString type() const override { return QStringLiteral("ToolCard"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;
};

/**
 * @class ThinkingComponent
 * @brief 思考过程组件
 *
 * 绘制 AI 思考过程的折叠面板。
 *
 * ## 布局
 * ┌─────────────────────────────────┐
 * │ [💡] Thinking...        [▼/▶] │
 * ├─────────────────────────────────┤
 * │ Step 1: ...                   │ (可折叠)
 * │ Step 2: ...                   │
 * └─────────────────────────────────┘
 *
 * ## 功能
 * - 折叠/展开控制
 * - 步骤列表显示
 * - 思考中动画
 */
class ThinkingComponent : public VLayout::AbstractComponent
{
public:
    explicit ThinkingComponent(const QString& id);

    QString type() const override { return QStringLiteral("Thinking"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;
};

/**
 * @class TaskListComponent
 * @brief 任务列表组件
 *
 * 绘制任务列表及其进度。
 *
 * ## 布局
 * ┌─────────────────────────────────┐
 * │ Tasks (3/5)           60% ████ │
 * ├─────────────────────────────────┤
 * │ ☑ Task 1 - completed          │
 * │ ▶ Task 2 - in progress        │
 * │ ○ Task 3 - pending            │
 * │   └─ depends on: Task 2       │
 * │ ○ Task 4 - pending            │
 * └─────────────────────────────────┘
 *
 * ## 功能
 * - 任务状态显示（待处理/进行中/已完成/失败）
 * - 任务依赖关系展示
 * - 折叠/展开控制
 * - 进度条显示
 */
class TaskListComponent : public VLayout::AbstractComponent
{
public:
    explicit TaskListComponent(const QString& id);

    QString type() const override { return QStringLiteral("TaskList"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;

    void drawTaskItem(VLayout::ComponentContext& ctx, const QRect& rect,
                      const QString& task, TaskStatus status, int indent);
    void drawProgressBar(QPainter* painter, const QRect& rect, int progress);
};

/**
 * @class CopyButtonComponent
 * @brief 复制按钮组件
 *
 * 绘制可交互的复制按钮。
 */
class CopyButtonComponent : public VLayout::AbstractComponent
{
public:
    explicit CopyButtonComponent(const QString& id);

    QString type() const override { return QStringLiteral("CopyButton"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;
};

/**
 * @class StatusBadgeComponent
 * @brief 状态徽章组件
 *
 * 绘制状态指示徽章。
 */
class StatusBadgeComponent : public VLayout::AbstractComponent
{
public:
    explicit StatusBadgeComponent(const QString& id);

    QString type() const override { return QStringLiteral("StatusBadge"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;

private:
    QSize m_sizeHint;
};

} // namespace Timeline

#endif // TIMELINE_COMPONENTS_H
