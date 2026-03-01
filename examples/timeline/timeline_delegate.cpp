#include "timeline_delegate.h"
#include "timeline_components.h"
#include "timeline_theme.h"
#include "timeline_utils.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QTimer>

namespace Timeline {

// ============================================================================
// TimelineDelegate
// ============================================================================

TimelineDelegate::TimelineDelegate(QObject* parent)
    : VLayout::DelegateController(parent)
{
}

void TimelineDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    // 扁平化设计：不绘制 hover 背景
    painter->save();
    painter->fillRect(option.rect, Theme::bgBase);
    painter->restore();

    // 绘制时间线（在整个 item 高度上）
    paintTimeline(painter, option, index);

    // 获取当前节点类型
    NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());

    // 确保布局已缓存（只在类型改变时重建）
    ensureLayoutCached(type, index);

    // 更新动态属性（如 copied 状态）
    updateDynamicProperties(index);

    // 调用父类绘制组件
    DelegateController::paint(painter, option, index);
}

void TimelineDelegate::paintTimeline(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    QRect r = option.rect;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 时间线位置
    int centerX = Theme::timelineX;
    int dotY = r.y() + r.height() / 2;

    // 获取圆点颜色
    NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());
    QString toolName = index.data(ToolNameRole).toString();
    QColor dotColor = Theme::getTimelineDotColor(static_cast<int>(type), toolName);

    // 绘制竖线轨道（贯穿整个 item）
    painter->setPen(QPen(Theme::timelineTrack, 1));
    painter->drawLine(centerX, r.y(), centerX, r.bottom());

    // 绘制圆点
    painter->setBrush(dotColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPoint(centerX, dotY), Theme::timelineDotRadius, Theme::timelineDotRadius);

    painter->restore();
}

QSize TimelineDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    int width = option.rect.width();
    if (width <= 0) {
        width = 400;
    }

    NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());
    int height = 40;  // 默认高度

    switch (type) {
    case NodeType::UserMessage:
    case NodeType::AIMessage: {
        QString content = index.data(ContentRole).toString();
        height = calculateMessageHeight(content, width - Theme::totalHMargin);
        height = qMax(40, height + 24);
        break;
    }
    case NodeType::CodeBlock: {
        QString code = index.data(CodeRole).toString();
        height = calculateCodeBlockHeight(code, width - Theme::totalHMargin);
        height = qMax(80, height + 28);
        break;
    }
    case NodeType::ToolCall: {
        height = calculateToolCallHeight(index, width - Theme::totalHMargin);
        break;
    }
    case NodeType::ThinkingStep: {
        QStringList steps = index.data(ThinkingStepsRole).toStringList();
        bool expanded = index.data(IsExpandedRole).toBool();
        height = calculateThinkingHeight(steps, expanded);
        break;
    }
    case NodeType::TaskList: {
        QVariantList tasks = index.data(TasksRole).toList();
        bool expanded = index.data(IsExpandedRole).toBool();
        height = calculateTaskListHeight(tasks, expanded);
        break;
    }
    }

    return QSize(width, height);
}

bool TimelineDelegate::editorEvent(QEvent* event,
                                    QAbstractItemModel* model,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();

        // 检测展开按钮点击
        QRect expandRect = getExpandButtonRect(option.rect);
        if (expandRect.contains(pos)) {
            QString nodeId = index.data(NodeIdRole).toString();
            bool expanded = index.data(IsExpandedRole).toBool();
            model->setData(index, !expanded, IsExpandedRole);
            emit expandedChanged(nodeId, !expanded);
            return true;
        }

        // 检测复制按钮点击（代码块）
        NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());
        if (type == NodeType::CodeBlock) {
            QRect copyRect = getCopyButtonRect(option.rect);
            if (copyRect.contains(pos)) {
                QString nodeId = index.data(NodeIdRole).toString();
                QString code = index.data(CodeRole).toString();

                // 复制到剪贴板
                QApplication::clipboard()->setText(code);
                m_copiedStates[nodeId] = true;

                emit copyCodeRequested(nodeId, code);

                // 2秒后重置复制状态
                QTimer::singleShot(2000, this, [this, nodeId]() {
                    m_copiedStates[nodeId] = false;
                });

                return true;
            }
        }
    }

    return DelegateController::editorEvent(event, model, option, index);
}

// ============================================================================
// 布局缓存
// ============================================================================

void TimelineDelegate::ensureLayoutCached(NodeType type, const QModelIndex& index) const
{
    // 类型相同时复用现有布局
    if (m_layoutCached && m_lastType == type) {
        return;
    }

    // 清除之前的布局并重建
    clearComponents();
    m_lastType = type;
    m_layoutCached = true;
    m_lastNodeId = index.data(NodeIdRole).toString();

    switch (type) {
    case NodeType::UserMessage:
        setupUserMessageLayout(index);
        break;
    case NodeType::AIMessage:
        setupAIMessageLayout(index);
        break;
    case NodeType::CodeBlock:
        setupCodeBlockLayout(index);
        break;
    case NodeType::ToolCall:
        setupToolCallLayout(index);
        break;
    case NodeType::ThinkingStep:
        setupThinkingLayout(index);
        break;
    case NodeType::TaskList:
        setupTaskListLayout(index);
        break;
    }
}

void TimelineDelegate::updateDynamicProperties(const QModelIndex& index) const
{
    NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());

    // 更新 CodeBlock 的 copied 状态（这是动态的，不在 model 中）
    if (type == NodeType::CodeBlock) {
        QString nodeId = index.data(NodeIdRole).toString();
        bool copied = m_copiedStates.value(nodeId, false);

        // 直接设置组件属性
        auto* comp = component(QStringLiteral("codeblock"));
        if (comp) {
            comp->setProperty("copied", copied);
        }
    }
}

// ============================================================================
// 布局设置
// ============================================================================

void TimelineDelegate::setupLayoutForType(const QModelIndex& index) const
{
    NodeType type = static_cast<NodeType>(index.data(NodeTypeRole).toInt());
    ensureLayoutCached(type, index);
}

void TimelineDelegate::setupUserMessageLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);
    setSpacing(8);

    addItem<MessageContentComponent>("content", -1);

    bindTo("content")
        .property("content", ContentRole)
        .property("isStreaming", false);
}

void TimelineDelegate::setupAIMessageLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);
    setSpacing(8);

    addItem<MessageContentComponent>("content", -1);

    bindTo("content")
        .property("content", ContentRole)
        .property("isStreaming", IsStreamingRole);
}

void TimelineDelegate::setupCodeBlockLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);

    addItem<CodeBlockComponent>("codeblock", -1);

    bindTo("codeblock")
        .property("language", LanguageRole)
        .property("code", CodeRole)
        .property("copyHovered", false)
        .property("copied", false);  // 初始值，动态更新在 updateDynamicProperties 中
}

void TimelineDelegate::setupToolCallLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);

    addItem<ToolCardComponent>("toolcard", -1);

    bindTo("toolcard")
        .property("toolName", ToolNameRole)
        .property("args", ToolArgsRole)
        .property("result", ToolResultRole)
        .property("status", ToolStatusRole)
        .property("expanded", IsExpandedRole);
}

void TimelineDelegate::setupThinkingLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);

    addItem<ThinkingComponent>("thinking", -1);

    bindTo("thinking")
        .property("steps", ThinkingStepsRole)
        .property("expanded", IsExpandedRole)
        .property("isThinking", false);
}

void TimelineDelegate::setupTaskListLayout(const QModelIndex& index) const
{
    Q_UNUSED(index)

    setMargins(Theme::contentLeftMargin, Theme::contentVMargin,
               Theme::contentRightMargin, Theme::contentVMargin);

    addItem<TaskListComponent>("tasklist", -1);

    bindTo("tasklist")
        .property("tasks", TasksRole)
        .property("completed", CompletedCountRole)
        .property("total", TaskCountRole)
        .property("expanded", IsExpandedRole);
}

// ============================================================================
// 尺寸计算
// ============================================================================

int TimelineDelegate::calculateMessageHeight(const QString& content,
                                              int width) const
{
    QFont font(QStringLiteral("Segoe UI"), 10);
    return Utils::calculateTextHeight(content, width, font);
}

int TimelineDelegate::calculateCodeBlockHeight(const QString& code,
                                                int width) const
{
    Q_UNUSED(width)

    QFont font(QStringLiteral("Consolas"), 10);
    QFontMetrics fm(font);
    int lineCount = code.count(QLatin1Char('\n')) + 1;
    int lineHeight = fm.height();

    // 头部(28) + 代码区(行数*行高 + 边距)
    return 28 + lineCount * lineHeight + 16;
}

int TimelineDelegate::calculateToolCallHeight(const QModelIndex& index,
                                               int width) const
{
    Q_UNUSED(width)

    bool expanded = index.data(IsExpandedRole).toBool();
    int height = 44;  // 头部高度

    if (expanded) {
        QVariantMap args = index.data(ToolArgsRole).toMap();
        QVariantMap result = index.data(ToolResultRole).toMap();

        if (!args.isEmpty()) {
            height += 20;
            QString argsText = Utils::formatToolArgs(args);
            height += argsText.count(QLatin1Char('\n')) * 14 + 20;
        }

        if (!result.isEmpty()) {
            height += 28;
            QString resultText = Utils::formatToolArgs(result);
            height += resultText.count(QLatin1Char('\n')) * 14 + 20;
        }
    }

    return height;
}

int TimelineDelegate::calculateThinkingHeight(const QStringList& steps,
                                               bool expanded) const
{
    int height = 44;  // 头部高度

    if (expanded && !steps.isEmpty()) {
        height += steps.size() * 20 + 16;
    }

    return height;
}

int TimelineDelegate::calculateTaskListHeight(const QVariantList& tasks,
                                               bool expanded) const
{
    int height = 48;  // 头部高度

    if (expanded && !tasks.isEmpty()) {
        height += tasks.size() * 28 + 8;
    }

    return height;
}

// ============================================================================
// 交互区域检测
// ============================================================================

QRect TimelineDelegate::getExpandButtonRect(const QRect& itemRect) const
{
    return QRect(itemRect.right() - 32, itemRect.top() + 8, 24, 24);
}

QRect TimelineDelegate::getCopyButtonRect(const QRect& itemRect) const
{
    return QRect(itemRect.right() - 52, itemRect.top() + 4, 24, 24);
}

} // namespace Timeline
