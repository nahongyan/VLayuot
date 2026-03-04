#ifndef CODE_HIGHLIGHTER_H
#define CODE_HIGHLIGHTER_H

/**
 * @file code_highlighter.h
 * @brief 代码语法高亮器
 *
 * 提供简单的语法高亮功能，无需外部依赖。
 * 支持常见语言的关键词、字符串、注释高亮。
 */

#include <QString>
#include <QStringList>
#include <QVector>
#include <QColor>
#include <QFont>
#include <QRegularExpression>
#include <QPixmap>
#include <QHash>

class QPainter;
class QRect;

namespace Timeline {

/**
 * @class CodeHighlighter
 * @brief 简单的代码语法高亮器
 *
 * 基于正则表达式的语法高亮，支持多种编程语言。
 * 内置缓存机制提升性能。
 *
 * @note 此类仅限在主线程使用，静态缓存非线程安全。
 */
class CodeHighlighter
{
public:
    /// 高亮片段
    struct Fragment {
        int start;
        int length;
        QColor color;
        bool bold = false;
    };

    /// 语言定义
    struct LanguageDef {
        QString name;
        QStringList keywords;
        QStringList types;
        QString singleLineComment;
        QColor keywordColor;
        QColor typeColor;
        QColor stringColor;
        QColor commentColor;
        QColor numberColor;
        QColor functionColor;
    };

    /**
     * @brief 获取语言定义
     * @param language 语言名称
     * @return 语言定义结构
     */
    static LanguageDef getLanguageDef(const QString& language);

    /**
     * @brief 高亮一行代码
     * @param line 代码行
     * @param langDef 语言定义
     * @return 高亮片段列表
     */
    static QVector<Fragment> highlightLine(const QString& line, const LanguageDef& langDef);

    /**
     * @brief 绘制高亮代码（带缓存）
     * @param painter 绘制器
     * @param rect 绘制区域
     * @param code 代码内容
     * @param language 语言
     * @param font 代码字体
     */
    static void drawCode(QPainter* painter, const QRect& rect,
                         const QString& code, const QString& language,
                         const QFont& font);

    /// 清除缓存
    static void clearCache();

    /// 设置缓存最大数量
    static void setMaxCacheSize(int size) { s_maxCacheSize = size; }

private:
    /// 生成缓存键
    static QString generateCacheKey(const QString& code, const QString& language,
                                     const QFont& font, int width, qreal dpr = 1.0);

    /// 高亮字符串字面量
    static void highlightStrings(QVector<Fragment>& fragments, const QString& line,
                                 const QColor& color);

    /// 高亮数字
    static void highlightNumbers(QVector<Fragment>& fragments, const QString& line,
                                  const QColor& color);

    /// 高亮关键词
    static void highlightKeywords(QVector<Fragment>& fragments, const QString& line,
                                   const QStringList& keywords, const QColor& color, bool bold = false);

    /// 高亮注释
    static void highlightComments(QVector<Fragment>& fragments, const QString& line,
                                   const QString& pattern, const QColor& color);

    /// 合并重叠的片段
    static void mergeFragments(QVector<Fragment>& fragments);

    /// 直接绘制（不使用缓存）
    static void drawCodeDirect(QPainter* painter, const QRect& rect,
                                const QString& code, const LanguageDef& langDef,
                                const QFont& font);

    /// 缓存
    static QHash<QString, QPixmap> s_cache;
    static int s_maxCacheSize;
};

} // namespace Timeline

#endif // CODE_HIGHLIGHTER_H
