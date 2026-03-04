#include "markdown_renderer.h"
#include "timeline_theme.h"
#include "code_highlighter.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QRegularExpression>
#include <QFontMetrics>

namespace Timeline {

// ============================================================================
// MarkdownRenderer - 解析
// ============================================================================

QVector<MarkdownRenderer::Block> MarkdownRenderer::parse(const QString& markdown)
{
    QVector<Block> blocks;

    if (markdown.isEmpty()) {
        return blocks;
    }

    QStringList lines = markdown.split(QLatin1Char('\n'));
    int i = 0;

    while (i < lines.size()) {
        const QString& line = lines[i];

        // 检测代码块开始 ```
        if (line.startsWith(QStringLiteral("```"))) {
            Block codeBlock = parseCodeBlock(markdown, i);
            blocks.append(codeBlock);
            continue;
        }

        // 普通文本行，收集到下一个代码块或结束
        QString textContent;
        while (i < lines.size()) {
            const QString& currentLine = lines[i];
            if (currentLine.startsWith(QStringLiteral("```"))) {
                break;  // 代码块开始，停止收集
            }
            if (!textContent.isEmpty()) {
                textContent += QLatin1Char('\n');
            }
            textContent += currentLine;
            ++i;
        }

        if (!textContent.isEmpty()) {
            Block textBlock;
            textBlock.type = BlockType::Text;
            textBlock.content = textContent;
            blocks.append(textBlock);
        }
    }

    return blocks;
}

MarkdownRenderer::Block MarkdownRenderer::parseCodeBlock(const QString& markdown, int& pos)
{
    Block block;
    block.type = BlockType::CodeBlock;

    QStringList lines = markdown.split(QLatin1Char('\n'));

    // 边界检查
    if (pos < 0 || pos >= lines.size()) {
        return block;
    }

    // 解析语言（```cpp, ```python 等）
    QString firstLine = lines[pos];
    block.language = firstLine.mid(3).trimmed().toLower();
    if (block.language.isEmpty()) {
        block.language = QStringLiteral("text");
    }

    ++pos;  // 跳过 ``` 行

    // 收集代码内容直到 ```
    QStringList codeLines;
    while (pos < lines.size()) {
        const QString& line = lines[pos];
        if (line == QStringLiteral("```")) {
            ++pos;  // 跳过结束 ```
            break;
        }
        codeLines.append(line);
        ++pos;
    }

    block.content = codeLines.join(QLatin1Char('\n'));
    return block;
}

// ============================================================================
// MarkdownRenderer - 高度计算
// ============================================================================

int MarkdownRenderer::calculateHeight(const QString& markdown, const Config& config)
{
    if (markdown.isEmpty()) {
        return 0;
    }

    QVector<Block> blocks = parse(markdown);
    int totalHeight = 0;

    for (const Block& block : blocks) {
        totalHeight += calculateBlockHeight(block, config);
        totalHeight += 8;  // 块间距
    }

    return qMax(0, totalHeight - 8);  // 减去最后一个块间距
}

int MarkdownRenderer::calculateBlockHeight(const Block& block, const Config& config)
{
    switch (block.type) {
    case BlockType::Text: {
        QTextDocument doc;
        setupDocument(&doc, block.content, config.width, config.textFont);
        return doc.size().height();
    }

    case BlockType::CodeBlock: {
        QFontMetrics fm(config.codeFont);
        int lineCount = block.content.count(QLatin1Char('\n')) + 1;
        int lineHeight = fm.height();

        // 头部 (28) + 代码行 + 底部边距 (16)
        int headerHeight = 28;
        int codeHeight = lineCount * lineHeight;
        int padding = config.codePadding * 2;

        return headerHeight + codeHeight + padding + 4;
    }

    default:
        return 24;
    }
}

// ============================================================================
// MarkdownRenderer - 渲染
// ============================================================================

int MarkdownRenderer::render(QPainter* painter, const QRect& rect,
                              const QString& markdown, const Config& config,
                              QVector<CodeBlockInfo>* codeBlocks)
{
    if (markdown.isEmpty()) {
        return 0;
    }

    if (codeBlocks) {
        codeBlocks->clear();
    }

    QVector<Block> blocks = parse(markdown);
    int yOffset = 0;

    for (const Block& block : blocks) {
        int blockHeight = renderBlock(painter, rect, block, config, yOffset, codeBlocks);
        yOffset += blockHeight;
        yOffset += 8;  // 块间距
    }

    return qMax(0, yOffset - 8);
}

int MarkdownRenderer::renderBlock(QPainter* painter, const QRect& rect,
                                   const Block& block, const Config& config, int yOffset,
                                   QVector<CodeBlockInfo>* codeBlocks)
{
    switch (block.type) {
    case BlockType::Text: {
        // 创建并渲染 QTextDocument
        QTextDocument doc;
        setupDocument(&doc, block.content, rect.width(), config.textFont);

        painter->save();
        painter->translate(rect.x(), rect.y() + yOffset);

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, config.textColor);
        doc.documentLayout()->draw(painter, ctx);

        painter->restore();

        return doc.size().height();
    }

    case BlockType::CodeBlock: {
        int blockHeight = calculateBlockHeight(block, config);
        QRect codeRect(rect.x(), rect.y() + yOffset, rect.width(), blockHeight);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        painter->setBrush(config.codeBackground);
        painter->setPen(QPen(config.codeBorderColor, 1));
        painter->drawRoundedRect(codeRect, config.codeBorderRadius, config.codeBorderRadius);

        // 绘制头部区域
        QRect headerRect(codeRect.x(), codeRect.y(), codeRect.width(), 28);
        painter->setPen(config.codeBorderColor);
        painter->drawLine(headerRect.bottomLeft(), headerRect.bottomRight());

        // 绘制语言标签
        QFont langFont(config.textFont);
        langFont.setPointSize(9);
        painter->setFont(langFont);
        painter->setPen(Theme::textSecond);
        QRect langRect(headerRect.x() + 12, headerRect.y(), 100, headerRect.height());
        painter->drawText(langRect, Qt::AlignLeft | Qt::AlignVCenter,
                          block.language.toLower());

        // 绘制复制按钮
        QRect copyBtnRect(codeRect.right() - 36, headerRect.y() + 4, 20, 20);
        drawCopyButton(painter, copyBtnRect, false, false);

        // 绘制代码内容（使用语法高亮）
        QRect codeContentRect(codeRect.x() + 12, headerRect.bottom() + config.codePadding,
                              codeRect.width() - 24, codeRect.height() - headerRect.height() - config.codePadding * 2 - 4);

        CodeHighlighter::drawCode(painter, codeContentRect, block.content,
                                   block.language, config.codeFont);

        painter->restore();

        // 记录代码块信息（用于交互）
        if (codeBlocks) {
            CodeBlockInfo info;
            info.bounds = codeRect;
            info.copyButtonRect = copyBtnRect;
            info.code = block.content;
            info.language = block.language;
            codeBlocks->append(info);
        }

        return blockHeight;
    }

    default:
        return 24;
    }
}

// ============================================================================
// MarkdownRenderer - 复制按钮
// ============================================================================

void MarkdownRenderer::drawCopyButton(QPainter* painter, const QRect& rect,
                                       bool hovered, bool copied)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 背景
    QColor bgColor = hovered ? QColor(80, 80, 80) : QColor(60, 60, 60);
    painter->setBrush(bgColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect, 3, 3);

    // 图标
    painter->setPen(copied ? Theme::success : Theme::textSecond);

    if (copied) {
        // 绘制对勾
        QPen checkPen(Theme::success, 2);
        checkPen.setCapStyle(Qt::RoundCap);
        checkPen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(checkPen);

        QPoint center = rect.center();
        QPainterPath checkPath;
        checkPath.moveTo(center.x() - 4, center.y());
        checkPath.lineTo(center.x() - 1, center.y() + 3);
        checkPath.lineTo(center.x() + 5, center.y() - 3);
        painter->drawPath(checkPath);
    } else {
        // 绘制复制图标（两个小矩形）
        QRect docRect(rect.x() + 5, rect.y() + 3, 8, 10);
        QRect clipRect(rect.x() + 7, rect.y() + 5, 8, 10);

        painter->drawRect(docRect);
        painter->drawRect(clipRect);
    }

    painter->restore();
}

// ============================================================================
// MarkdownRenderer - 文档设置
// ============================================================================

void MarkdownRenderer::setupDocument(QTextDocument* doc, const QString& markdown,
                                      int width, const QFont& font)
{
    doc->setDefaultFont(font);
    doc->setTextWidth(width);
    doc->setDocumentMargin(0);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Qt 5.14+ 支持 setMarkdown
    doc->setMarkdown(markdown);
#else
    // 回退到纯文本
    doc->setPlainText(markdown);
#endif
}

} // namespace Timeline
