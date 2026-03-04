#ifndef MARKDOWN_RENDERER_H
#define MARKDOWN_RENDERER_H

/**
 * @file markdown_renderer.h
 * @brief Markdown 渲染器
 *
 * 基于 QTextDocument 的 Markdown 渲染支持。
 */

#include <QString>
#include <QTextDocument>
#include <QVector>
#include <QRect>
#include <QColor>
#include <QFont>
#include <QHash>

class QPainter;

namespace Timeline {

/**
 * @class MarkdownRenderer
 * @brief Markdown 文本渲染器
 *
 * 使用 Qt 内置的 Markdown 支持 (Qt 5.14+) 渲染 Markdown 内容。
 * 支持常见格式：标题、粗体、斜体、链接、列表、代码块。
 */
class MarkdownRenderer
{
public:
    /// 内容块类型
    enum class BlockType {
        Text,       ///< 普通文本/Markdown 混合
        CodeBlock,  ///< 代码块
        InlineCode  ///< 内联代码
    };

    /// 解析后的内容块
    struct Block {
        BlockType type = BlockType::Text;
        QString content;
        QString language;       ///< 代码块语言（仅 CodeBlock 类型）
        int indentLevel = 0;    ///< 缩进级别
    };

    /// 渲染配置
    struct Config {
        int width = 400;                ///< 可用宽度
        QColor textColor;               ///< 文本颜色
        QColor codeBackground;           ///< 代码块背景
        QColor codeBorderColor;          ///< 代码块边框色
        QFont textFont;                  ///< 文本字体
        QFont codeFont;                  ///< 代码字体
        int codePadding = 8;             ///< 代码块内边距
        int codeBorderRadius = 6;        ///< 代码块圆角
    };

    /// 代码块信息（用于交互）
    struct CodeBlockInfo {
        QRect bounds;           ///< 代码块边界
        QRect copyButtonRect;   ///< 复制按钮区域
        QString code;           ///< 代码内容
        QString language;       ///< 语言
    };

    /**
     * @brief 解析 Markdown 内容为块列表
     * @param markdown Markdown 文本
     * @return 块列表
     */
    static QVector<Block> parse(const QString& markdown);

    /**
     * @brief 计算渲染高度
     * @param markdown Markdown 文本
     * @param config 渲染配置
     * @return 总高度
     */
    static int calculateHeight(const QString& markdown, const Config& config);

    /**
     * @brief 渲染 Markdown 到指定区域
     * @param painter 绘制器
     * @param rect 目标区域
     * @param markdown Markdown 文本
     * @param config 渲染配置
     * @param codeBlocks 输出：代码块信息列表（用于交互）
     * @return 实际渲染高度
     */
    static int render(QPainter* painter, const QRect& rect,
                      const QString& markdown, const Config& config,
                      QVector<CodeBlockInfo>* codeBlocks = nullptr);

    /**
     * @brief 设置 QTextDocument 的 Markdown 内容
     * @param doc 文档对象
     * @param markdown Markdown 文本
     * @param width 文档宽度
     * @param font 字体
     */
    static void setupDocument(QTextDocument* doc, const QString& markdown,
                              int width, const QFont& font);

private:
    /// 解析代码块（提取语言和内容）
    static Block parseCodeBlock(const QString& lines, int& pos);

    /// 计算单个块的高度
    static int calculateBlockHeight(const Block& block, const Config& config);

    /// 渲染单个块
    static int renderBlock(QPainter* painter, const QRect& rect,
                           const Block& block, const Config& config, int yOffset,
                           QVector<CodeBlockInfo>* codeBlocks);

    /// 绘制复制按钮
    static void drawCopyButton(QPainter* painter, const QRect& rect,
                                bool hovered, bool copied);
};

} // namespace Timeline

#endif // MARKDOWN_RENDERER_H
