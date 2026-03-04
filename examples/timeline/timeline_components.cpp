#include "timeline_components.h"
#include "timeline_theme.h"
#include "timeline_utils.h"
#include "markdown_renderer.h"
#include "code_highlighter.h"

#include <QFontMetrics>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

namespace Timeline {

// ============================================================================
// TimelineIndicatorComponent - 时间线指示器
// ============================================================================

TimelineIndicatorComponent::TimelineIndicatorComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_dotColor(Theme::dotDefault)
{
}

QSize TimelineIndicatorComponent::sizeHint() const
{
    return QSize(24, 20);  // 固定宽度 24px
}

void TimelineIndicatorComponent::paint(VLayout::ComponentContext& ctx)
{
    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    int centerX = r.x() + r.width() / 2;
    int dotY = r.y() + r.height() / 2;

    // 绘制完整竖线轨道（贯穿整个项目）
    ctx.painter->setPen(QPen(Theme::timelineTrack, 1));
    ctx.painter->drawLine(centerX, r.y(), centerX, r.bottom());

    // 绘制圆点（覆盖在线上）
    ctx.painter->setBrush(m_dotColor);
    ctx.painter->setPen(Qt::NoPen);
    ctx.painter->drawEllipse(QPoint(centerX, dotY), 4, 4);  // 8px 直径

    ctx.painter->restore();
}

// ============================================================================
// MessageContentComponent - 消息内容 (Markdown 渲染)
// ============================================================================

MessageContentComponent::MessageContentComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(300, 40)
{
}

QSize MessageContentComponent::sizeHint() const
{
    return m_sizeHint;
}

void MessageContentComponent::paint(VLayout::ComponentContext& ctx)
{
    QString content = property(QStringLiteral("content")).toString();
    bool isStreaming = property(QStringLiteral("isStreaming")).toBool();

    QRect r = geometry();
    ctx.painter->save();

    // 配置 Markdown 渲染器
    MarkdownRenderer::Config config;
    config.width = r.width();
    config.textColor = Theme::textPrimary;
    config.codeBackground = Theme::bgCodeBlock;
    config.codeBorderColor = Theme::codeBorder;
    config.textFont = Theme::textFont(10);
    config.codeFont = Theme::codeFont(10);
    config.codePadding = 8;
    config.codeBorderRadius = Theme::borderRadius;

    // 使用 Markdown 渲染器绘制内容，并获取代码块信息
    m_codeBlocks.clear();
    MarkdownRenderer::render(ctx.painter, r, content, config, &m_codeBlocks);

    // 流式输出指示器
    if (isStreaming) {
        int contentHeight = MarkdownRenderer::calculateHeight(content, config);
        QRect indicatorRect(r.x(), r.y() + contentHeight + 4, 6, 6);
        ctx.painter->setBrush(Theme::accentAI);
        ctx.painter->setPen(Qt::NoPen);
        ctx.painter->setRenderHint(QPainter::Antialiasing);
        ctx.painter->drawEllipse(indicatorRect);
    }

    ctx.painter->restore();

    // 更新 sizeHint
    int height = MarkdownRenderer::calculateHeight(content, config) + 8;
    if (isStreaming) {
        height += 10;  // 流式指示器空间
    }
    m_sizeHint.setHeight(qMax(24, height));
}

const MarkdownRenderer::CodeBlockInfo* MessageContentComponent::hitTestCopyButton(const QPoint& pos) const
{
    for (const auto& codeBlock : m_codeBlocks) {
        if (codeBlock.copyButtonRect.contains(pos)) {
            return &codeBlock;
        }
    }
    return nullptr;
}

// ============================================================================
// CodeBlockComponent
// ============================================================================

CodeBlockComponent::CodeBlockComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(300, 80)
{
}

QSize CodeBlockComponent::sizeHint() const
{
    return m_sizeHint;
}

void CodeBlockComponent::paint(VLayout::ComponentContext& ctx)
{
    QString language = property(QStringLiteral("language")).toString();
    QString code = property(QStringLiteral("code")).toString();
    bool copyHovered = property(QStringLiteral("copyHovered")).toBool();
    bool copied = property(QStringLiteral("copied")).toBool();

    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    // 背景 - 圆角
    ctx.painter->setBrush(Theme::bgCodeBlock);
    ctx.painter->setPen(QPen(Theme::codeBorder, 1));
    ctx.painter->drawRoundedRect(r, 6, 6);

    // 头部区域
    QRect headerRect(r.x(), r.y(), r.width(), 28);
    ctx.painter->setPen(Theme::separator);
    ctx.painter->drawLine(headerRect.bottomLeft(), headerRect.bottomRight());

    // 语言标签
    QFont langFont = Theme::textFont(9);
    ctx.painter->setFont(langFont);
    ctx.painter->setPen(Theme::textSecond);
    QRect langRect(headerRect.x() + 12, headerRect.y(), 100, headerRect.height());
    ctx.painter->drawText(langRect, Qt::AlignLeft | Qt::AlignVCenter, language.toLower());

    // 复制按钮
    QRect copyBtnRect(r.right() - 36, headerRect.y() + 4, 20, 20);
    Utils::drawCopyButton(ctx.painter, copyBtnRect, copyHovered, copied);

    // 代码区域（使用语法高亮）
    QRect codeRect(r.x() + 12, headerRect.bottom() + 8, r.width() - 24, r.height() - 44);
    QFont codeFont(QStringLiteral("Consolas"), 10);

    // 使用语法高亮绘制代码
    CodeHighlighter::drawCode(ctx.painter, codeRect, code, language, codeFont);

    ctx.painter->restore();
}

// ============================================================================
// ToolCardComponent
// ============================================================================

ToolCardComponent::ToolCardComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(300, 60)
{
}

QSize ToolCardComponent::sizeHint() const
{
    return m_sizeHint;
}

void ToolCardComponent::paint(VLayout::ComponentContext& ctx)
{
    QString toolName = property(QStringLiteral("toolName")).toString();
    QVariantMap args = property(QStringLiteral("args")).toMap();
    QVariantMap result = property(QStringLiteral("result")).toMap();
    int statusInt = property(QStringLiteral("status")).toInt();
    ToolStatus status = static_cast<ToolStatus>(statusInt);
    bool expanded = property(QStringLiteral("expanded")).toBool();

    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    ctx.painter->setBrush(Theme::bgToolCard);
    ctx.painter->setPen(QPen(Theme::toolBorder, 1));
    ctx.painter->drawRoundedRect(r, 6, 6);

    // 头部区域
    QRect headerRect(r.x(), r.y(), r.width(), 32);

    // 工具图标
    QRect iconRect(headerRect.x() + 8, headerRect.y() + 6, 20, 20);
    Utils::drawToolIcon(ctx.painter, iconRect, toolName, status);

    // 工具名称
    QFont nameFont = Theme::textFont(10);
    nameFont.setBold(true);
    ctx.painter->setFont(nameFont);
    ctx.painter->setPen(Theme::textPrimary);
    QRect nameRect(iconRect.right() + 8, headerRect.y(),
                    headerRect.width() - 100, headerRect.height());
    QString displayName = Utils::getToolDisplayName(toolName);
    ctx.painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);

    // 状态指示器
    QRect statusRect(headerRect.right() - 40, headerRect.y() + 10, 12, 12);
    Utils::drawStatusIndicator(ctx.painter, statusRect, status);

    // 展开/折叠箭头
    QRect arrowRect(headerRect.right() - 20, headerRect.y() + 6, 20, 20);
    Utils::drawExpandArrow(ctx.painter, arrowRect, expanded);

    // 展开内容
    if (expanded) {
        QFont codeFont(QStringLiteral("Consolas"), 9);
        ctx.painter->setFont(codeFont);
        ctx.painter->setPen(Theme::textSecond);

        int y = headerRect.bottom() + 12;

        // 参数
        if (!args.isEmpty()) {
            ctx.painter->drawText(r.x() + 12, y, QStringLiteral("Args:"));
            y += 16;
            QString argsText = Utils::formatToolArgs(args);
            QStringList argLines = argsText.split(QLatin1Char('\n'));
            for (const QString& line : argLines) {
                if (y > r.bottom() - 12) break;
                ctx.painter->drawText(r.x() + 24, y,
                                  Utils::truncateText(line, r.width() - 36));
                y += 14;
            }
        }

        // 结果
        if (!result.isEmpty() && status == ToolStatus::Success) {
            y += 8;
            ctx.painter->drawText(r.x() + 12, y, QStringLiteral("Result:"));
            y += 16;
            QString resultText = Utils::formatToolArgs(result);
            QStringList resultLines = resultText.split(QLatin1Char('\n'));
            for (const QString& line : resultLines) {
                if (y > r.bottom() - 12) break;
                ctx.painter->drawText(r.x() + 24, y,
                                  Utils::truncateText(line, r.width() - 36));
                y += 14;
            }
        }
    }

    ctx.painter->restore();
}

// ============================================================================
// ThinkingComponent
// ============================================================================

ThinkingComponent::ThinkingComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(300, 40)
{
}

QSize ThinkingComponent::sizeHint() const
{
    return m_sizeHint;
}

void ThinkingComponent::paint(VLayout::ComponentContext& ctx)
{
    QStringList steps = property(QStringLiteral("steps")).toStringList();
    bool expanded = property(QStringLiteral("expanded")).toBool();
    bool isThinking = property(QStringLiteral("isThinking")).toBool();

    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    ctx.painter->setBrush(Theme::bgThinking);
    ctx.painter->setPen(QPen(Theme::separator, 1));
    ctx.painter->drawRoundedRect(r, 6, 6);

    // 头部
    QRect headerRect(r.x(), r.y(), r.width(), 32);

    // 思考图标
    QRect iconRect(headerRect.x() + 8, headerRect.y() + 6, 20, 20);
    ctx.painter->setBrush(Theme::dotThinking);
    ctx.painter->setPen(Qt::NoPen);
    ctx.painter->drawEllipse(iconRect);

    ctx.painter->setPen(QColor(30, 30, 30));
    QFont iconFont = Theme::textFont(9);
    ctx.painter->setFont(iconFont);
    ctx.painter->drawText(iconRect, Qt::AlignCenter,
                      isThinking ? QStringLiteral("...") : QStringLiteral("T"));

    // 标题
    QFont titleFont = Theme::textFont(10);
    ctx.painter->setFont(titleFont);
    ctx.painter->setPen(Theme::textPrimary);
    QString title = isThinking ? QStringLiteral("Thinking...")
                               : QStringLiteral("Thought Process (%1 steps)").arg(steps.size());
    QRect titleRect(iconRect.right() + 8, headerRect.y(),
                     headerRect.width() - 80, headerRect.height());
    ctx.painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

    // 展开/折叠箭头
    QRect arrowRect(headerRect.right() - 24, headerRect.y() + 6, 20, 20);
    Utils::drawExpandArrow(ctx.painter, arrowRect, expanded);

    // 计算内容高度
    int contentHeight = 32;  // 头部高度

    // 展开步骤
    if (expanded && !steps.isEmpty()) {
        QFont stepFont = Theme::textFont(9);
        ctx.painter->setFont(stepFont);
        ctx.painter->setPen(Theme::textSecond);

        int y = headerRect.bottom() + 12;
        int maxWidth = r.width() - 48;
        int stepNum = 1;

        for (const QString& step : steps) {
            // 绘制步骤编号
            QString numText = QStringLiteral("%1. ").arg(stepNum++);
            QFontMetrics fm(stepFont);
            int numWidth = fm.horizontalAdvance(numText);
            ctx.painter->drawText(r.x() + 24, y + fm.ascent(), numText);

            // 使用 QTextDocument 进行自动换行（宽度要减去编号宽度）
            QTextDocument doc;
            doc.setDefaultFont(stepFont);
            doc.setTextWidth(maxWidth - numWidth);
            doc.setDocumentMargin(0);
            doc.setPlainText(step);

            // 绘制步骤内容（自动换行）
            ctx.painter->save();
            ctx.painter->translate(r.x() + 24 + numWidth, y);
            QAbstractTextDocumentLayout::PaintContext paintCtx;
            paintCtx.palette.setColor(QPalette::Text, Theme::textSecond);
            doc.documentLayout()->draw(ctx.painter, paintCtx);
            ctx.painter->restore();

            int stepHeight = doc.size().height() + 8;  // 步骤间距
            y += stepHeight;
            contentHeight = y - r.y() + 8;  // 底部边距
        }
    }

    ctx.painter->restore();

    // 更新 sizeHint
    m_sizeHint.setHeight(qMax(40, contentHeight));
}

// ============================================================================
// TaskListComponent
// ============================================================================

TaskListComponent::TaskListComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(300, 100)
{
}

QSize TaskListComponent::sizeHint() const
{
    return m_sizeHint;
}

void TaskListComponent::paint(VLayout::ComponentContext& ctx)
{
    QVariantList tasks = property(QStringLiteral("tasks")).toList();
    int completed = property(QStringLiteral("completed")).toInt();
    int total = property(QStringLiteral("total")).toInt();
    bool expanded = property(QStringLiteral("expanded")).toBool();

    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    ctx.painter->setBrush(Theme::bgSurface);
    ctx.painter->setPen(QPen(Theme::separator, 1));
    ctx.painter->drawRoundedRect(r, 6, 6);

    // 头部区域
    QRect headerRect(r.x(), r.y(), r.width(), 36);

    // 任务图标
    QRect iconRect(headerRect.x() + 8, headerRect.y() + 8, 20, 20);
    ctx.painter->setBrush(Theme::accentAI);
    ctx.painter->setPen(Qt::NoPen);
    ctx.painter->drawRoundedRect(iconRect, 3, 3);
    ctx.painter->setPen(Qt::white);
    QFont iconFont = Theme::textFont(10);
    ctx.painter->setFont(iconFont);
    ctx.painter->drawText(iconRect, Qt::AlignCenter, QStringLiteral("X"));

    // 标题和进度
    QFont titleFont = Theme::textFont(10);
    titleFont.setBold(true);
    ctx.painter->setFont(titleFont);
    ctx.painter->setPen(Theme::textPrimary);
    QString title = QStringLiteral("Tasks (%1/%2)").arg(completed).arg(total);
    QRect titleRect(iconRect.right() + 8, headerRect.y(),
                     headerRect.width() - 150, headerRect.height());
    ctx.painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);

    // 进度条
    int progress = total > 0 ? (completed * 100 / total) : 0;
    QRect progressRect(headerRect.right() - 100, headerRect.y() + 10, 60, 16);
    drawProgressBar(ctx.painter, progressRect, progress);

    // 展开/折叠箭头
    QRect arrowRect(headerRect.right() - 24, headerRect.y() + 8, 20, 20);
    Utils::drawExpandArrow(ctx.painter, arrowRect, expanded);

    // 展开任务列表
    if (expanded && !tasks.isEmpty()) {
        int y = headerRect.bottom() + 8;
        for (const QVariant& taskVar : tasks) {
            if (y > r.bottom() - 8) break;

            QVariantMap task = taskVar.toMap();
            QString taskName = task.value(QStringLiteral("name")).toString();
            int statusInt = task.value(QStringLiteral("status")).toInt();
            TaskStatus status = static_cast<TaskStatus>(statusInt);
            int indent = task.value(QStringLiteral("indent")).toInt();

            QRect taskRect(r.x() + 8, y, r.width() - 16, 24);
            drawTaskItem(ctx, taskRect, taskName, status, indent);
            y += 26;
        }
    }

    ctx.painter->restore();
}

void TaskListComponent::drawTaskItem(VLayout::ComponentContext& ctx,
                                      const QRect& rect,
                                      const QString& task,
                                      TaskStatus status,
                                      int indent)
{
    QRect r = rect.adjusted(indent * 16, 0, 0, 0);

    // 状态图标
    QString icon;
    QColor iconColor;
    switch (status) {
    case TaskStatus::Completed:
        icon = QStringLiteral("[X]");
        iconColor = Theme::success;
        break;
    case TaskStatus::InProgress:
        icon = QStringLiteral("[>]");
        iconColor = Theme::running;
        break;
    case TaskStatus::Failed:
        icon = QStringLiteral("[!]");
        iconColor = Theme::error;
        break;
    case TaskStatus::Skipped:
        icon = QStringLiteral("[-]");
        iconColor = Theme::textSecond;
        break;
    case TaskStatus::Pending:
    default:
        icon = QStringLiteral("[ ]");
        iconColor = Theme::textSecond;
        break;
    }

    ctx.painter->setPen(iconColor);
    QFont iconFont(QStringLiteral("Consolas"), 9);
    ctx.painter->setFont(iconFont);
    ctx.painter->drawText(r.x(), r.y() + 16, icon);

    // 任务文本
    ctx.painter->setPen(status == TaskStatus::Completed ? Theme::textSecond
                                                        : Theme::textPrimary);
    QFont textFont = Theme::textFont(9);
    ctx.painter->setFont(textFont);
    QString text = Utils::truncateText(task, r.width() - 50);
    ctx.painter->drawText(r.x() + 30, r.y() + 16, text);
}

void TaskListComponent::drawProgressBar(QPainter* painter,
                                         const QRect& rect,
                                         int progress)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    painter->setBrush(Theme::separator);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect, 3, 3);

    // 进度
    if (progress > 0) {
        int fillWidth = rect.width() * progress / 100;
        QRect fillRect(rect.x(), rect.y(), fillWidth, rect.height());
        painter->setBrush(Theme::accentAI);
        painter->drawRoundedRect(fillRect, 3, 3);
    }

    // 百分比文本
    painter->setPen(Theme::textPrimary);
    QFont font = Theme::textFont(8);
    painter->setFont(font);
    QString text = QStringLiteral("%1%").arg(progress);
    painter->drawText(rect, Qt::AlignCenter, text);

    painter->restore();
}

// ============================================================================
// CopyButtonComponent
// ============================================================================

CopyButtonComponent::CopyButtonComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(24, 24)
{
}

QSize CopyButtonComponent::sizeHint() const
{
    return m_sizeHint;
}

void CopyButtonComponent::paint(VLayout::ComponentContext& ctx)
{
    bool hovered = hasState(VLayout::ComponentState::Hovered);
    bool copied = property(QStringLiteral("copied")).toBool();

    Utils::drawCopyButton(ctx.painter, geometry(), hovered, copied);
}

// ============================================================================
// StatusBadgeComponent
// ============================================================================

StatusBadgeComponent::StatusBadgeComponent(const QString& id)
    : VLayout::AbstractComponent(id)
    , m_sizeHint(60, 20)
{
}

QSize StatusBadgeComponent::sizeHint() const
{
    return m_sizeHint;
}

void StatusBadgeComponent::paint(VLayout::ComponentContext& ctx)
{
    int statusInt = property(QStringLiteral("status")).toInt();
    ToolStatus status = static_cast<ToolStatus>(statusInt);

    QString text;
    QColor bgColor;
    switch (status) {
    case ToolStatus::Pending:
        text = QStringLiteral("Pending");
        bgColor = Theme::textSecond;
        break;
    case ToolStatus::Running:
        text = QStringLiteral("Running");
        bgColor = Theme::running;
        break;
    case ToolStatus::Success:
        text = QStringLiteral("Success");
        bgColor = Theme::success;
        break;
    case ToolStatus::Error:
        text = QStringLiteral("Error");
        bgColor = Theme::error;
        break;
    default:
        text = QStringLiteral("Unknown");
        bgColor = Theme::textSecond;
        break;
    }

    QRect r = geometry();
    ctx.painter->save();
    ctx.painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    ctx.painter->setBrush(bgColor);
    ctx.painter->setPen(Qt::NoPen);
    ctx.painter->drawRoundedRect(r, 3, 3);

    // 文本
    ctx.painter->setPen(Qt::white);
    QFont font = Theme::textFont(8);
    ctx.painter->setFont(font);
    ctx.painter->drawText(r, Qt::AlignCenter, text);

    ctx.painter->restore();
}

} // namespace Timeline
