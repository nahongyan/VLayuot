#include "code_highlighter.h"
#include "timeline_theme.h"

#include <QPainter>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <algorithm>

namespace Timeline {

// 静态成员初始化
QHash<QString, QPixmap> CodeHighlighter::s_cache;
int CodeHighlighter::s_maxCacheSize = 50;

// ============================================================================
// CodeHighlighter - 缓存管理
// ============================================================================

QString CodeHighlighter::generateCacheKey(const QString& code, const QString& language,
                                            const QFont& font, int width, qreal dpr)
{
    // 使用哈希生成唯一键（包含字体族、大小和 DPR）
    QString keyData = language + QStringLiteral("|") +
                      QString::number(width) + QStringLiteral("|") +
                      font.family() + QStringLiteral("|") +
                      QString::number(font.pointSize()) + QStringLiteral("|") +
                      QString::number(font.weight()) + QStringLiteral("|") +
                      QString::number(dpr, 'f', 2) + QStringLiteral("|") +
                      code;
    return QCryptographicHash::hash(keyData.toUtf8(), QCryptographicHash::Md5).toHex();
}

void CodeHighlighter::clearCache()
{
    s_cache.clear();
}

// ============================================================================
// CodeHighlighter - 语言定义
// ============================================================================

CodeHighlighter::LanguageDef CodeHighlighter::getLanguageDef(const QString& language)
{
    LanguageDef def;
    def.name = language.toLower();

    // 默认颜色（使用 Theme）
    def.keywordColor = Theme::syntaxKeyword;
    def.typeColor = Theme::syntaxType;
    def.stringColor = Theme::syntaxString;
    def.commentColor = Theme::syntaxComment;
    def.numberColor = Theme::syntaxNumber;
    def.functionColor = Theme::syntaxFunction;

    // 单行注释默认
    def.singleLineComment = QStringLiteral("//");

    QString lang = language.toLower();

    if (lang == QStringLiteral("cpp") || lang == QStringLiteral("c++") ||
        lang == QStringLiteral("c") || lang == QStringLiteral("h")) {
        def.keywords << QStringLiteral("auto") << QStringLiteral("break") << QStringLiteral("case")
                     << QStringLiteral("class") << QStringLiteral("const") << QStringLiteral("continue")
                     << QStringLiteral("default") << QStringLiteral("delete") << QStringLiteral("do")
                     << QStringLiteral("else") << QStringLiteral("enum") << QStringLiteral("explicit")
                     << QStringLiteral("extern") << QStringLiteral("false") << QStringLiteral("for")
                     << QStringLiteral("friend") << QStringLiteral("goto") << QStringLiteral("if")
                     << QStringLiteral("inline") << QStringLiteral("namespace") << QStringLiteral("new")
                     << QStringLiteral("operator") << QStringLiteral("private") << QStringLiteral("protected")
                     << QStringLiteral("public") << QStringLiteral("return") << QStringLiteral("sizeof")
                     << QStringLiteral("static") << QStringLiteral("struct") << QStringLiteral("switch")
                     << QStringLiteral("template") << QStringLiteral("this") << QStringLiteral("throw")
                     << QStringLiteral("true") << QStringLiteral("try") << QStringLiteral("typedef")
                     << QStringLiteral("typename") << QStringLiteral("union") << QStringLiteral("using")
                     << QStringLiteral("virtual") << QStringLiteral("void") << QStringLiteral("volatile")
                     << QStringLiteral("while") << QStringLiteral("nullptr") << QStringLiteral("override")
                     << QStringLiteral("final") << QStringLiteral("constexpr") << QStringLiteral("noexcept");

        def.types << QStringLiteral("int") << QStringLiteral("char") << QStringLiteral("short")
                  << QStringLiteral("long") << QStringLiteral("float") << QStringLiteral("double")
                  << QStringLiteral("bool") << QStringLiteral("void") << QStringLiteral("wchar_t")
                  << QStringLiteral("char16_t") << QStringLiteral("char32_t") << QStringLiteral("auto")
                  << QStringLiteral("QString") << QStringLiteral("QByteArray") << QStringLiteral("QVariant")
                  << QStringLiteral("QList") << QStringLiteral("QVector") << QStringLiteral("QMap")
                  << QStringLiteral("QHash") << QStringLiteral("QSet") << QStringLiteral("QObject")
                  << QStringLiteral("QWidget") << QStringLiteral("QColor") << QStringLiteral("QFont")
                  << QStringLiteral("QRect") << QStringLiteral("QPoint") << QStringLiteral("QSize")
                  << QStringLiteral("QPainter") << QStringLiteral("QPen") << QStringLiteral("QBrush")
                  << QStringLiteral("std::string") << QStringLiteral("std::vector") << QStringLiteral("std::map");

        def.singleLineComment = QStringLiteral("//");
    }
    else if (lang == QStringLiteral("python") || lang == QStringLiteral("py")) {
        def.keywords << QStringLiteral("False") << QStringLiteral("None") << QStringLiteral("True")
                     << QStringLiteral("and") << QStringLiteral("as") << QStringLiteral("assert")
                     << QStringLiteral("async") << QStringLiteral("await") << QStringLiteral("break")
                     << QStringLiteral("class") << QStringLiteral("continue") << QStringLiteral("def")
                     << QStringLiteral("del") << QStringLiteral("elif") << QStringLiteral("else")
                     << QStringLiteral("except") << QStringLiteral("finally") << QStringLiteral("for")
                     << QStringLiteral("from") << QStringLiteral("global") << QStringLiteral("if")
                     << QStringLiteral("import") << QStringLiteral("in") << QStringLiteral("is")
                     << QStringLiteral("lambda") << QStringLiteral("nonlocal") << QStringLiteral("not")
                     << QStringLiteral("or") << QStringLiteral("pass") << QStringLiteral("raise")
                     << QStringLiteral("return") << QStringLiteral("try") << QStringLiteral("while")
                     << QStringLiteral("with") << QStringLiteral("yield");

        def.types << QStringLiteral("int") << QStringLiteral("str") << QStringLiteral("bool")
                  << QStringLiteral("float") << QStringLiteral("list") << QStringLiteral("dict")
                  << QStringLiteral("set") << QStringLiteral("tuple") << QStringLiteral("bytes")
                  << QStringLiteral("object") << QStringLiteral("type") << QStringLiteral("None");

        def.singleLineComment = QStringLiteral("#");
    }
    else if (lang == QStringLiteral("javascript") || lang == QStringLiteral("js") ||
             lang == QStringLiteral("typescript") || lang == QStringLiteral("ts")) {
        def.keywords << QStringLiteral("async") << QStringLiteral("await") << QStringLiteral("break")
                     << QStringLiteral("case") << QStringLiteral("catch") << QStringLiteral("class")
                     << QStringLiteral("const") << QStringLiteral("continue") << QStringLiteral("debugger")
                     << QStringLiteral("default") << QStringLiteral("delete") << QStringLiteral("do")
                     << QStringLiteral("else") << QStringLiteral("export") << QStringLiteral("extends")
                     << QStringLiteral("false") << QStringLiteral("finally") << QStringLiteral("for")
                     << QStringLiteral("function") << QStringLiteral("if") << QStringLiteral("import")
                     << QStringLiteral("in") << QStringLiteral("instanceof") << QStringLiteral("let")
                     << QStringLiteral("new") << QStringLiteral("null") << QStringLiteral("return")
                     << QStringLiteral("static") << QStringLiteral("super") << QStringLiteral("switch")
                     << QStringLiteral("this") << QStringLiteral("throw") << QStringLiteral("true")
                     << QStringLiteral("try") << QStringLiteral("typeof") << QStringLiteral("undefined")
                     << QStringLiteral("var") << QStringLiteral("void") << QStringLiteral("while")
                     << QStringLiteral("with") << QStringLiteral("yield");

        def.types << QStringLiteral("string") << QStringLiteral("number") << QStringLiteral("boolean")
                  << QStringLiteral("object") << QStringLiteral("any") << QStringLiteral("void")
                  << QStringLiteral("never") << QStringLiteral("unknown") << QStringLiteral("interface")
                  << QStringLiteral("type") << QStringLiteral("enum");

        def.singleLineComment = QStringLiteral("//");
    }
    else if (lang == QStringLiteral("json")) {
        def.keywords << QStringLiteral("true") << QStringLiteral("false") << QStringLiteral("null");
        def.singleLineComment.clear();  // JSON 没有注释
    }
    else if (lang == QStringLiteral("bash") || lang == QStringLiteral("shell") ||
             lang == QStringLiteral("sh")) {
        def.keywords << QStringLiteral("if") << QStringLiteral("then") << QStringLiteral("else")
                     << QStringLiteral("elif") << QStringLiteral("fi") << QStringLiteral("for")
                     << QStringLiteral("while") << QStringLiteral("do") << QStringLiteral("done")
                     << QStringLiteral("case") << QStringLiteral("esac") << QStringLiteral("function")
                     << QStringLiteral("return") << QStringLiteral("exit") << QStringLiteral("break")
                     << QStringLiteral("continue") << QStringLiteral("local") << QStringLiteral("export")
                     << QStringLiteral("source") << QStringLiteral("echo") << QStringLiteral("cd")
                     << QStringLiteral("mkdir") << QStringLiteral("rm") << QStringLiteral("cp")
                     << QStringLiteral("mv");

        def.singleLineComment = QStringLiteral("#");
    }
    else if (lang == QStringLiteral("sql")) {
        def.keywords << QStringLiteral("SELECT") << QStringLiteral("FROM") << QStringLiteral("WHERE")
                     << QStringLiteral("INSERT") << QStringLiteral("INTO") << QStringLiteral("VALUES")
                     << QStringLiteral("UPDATE") << QStringLiteral("SET") << QStringLiteral("DELETE")
                     << QStringLiteral("CREATE") << QStringLiteral("TABLE") << QStringLiteral("DROP")
                     << QStringLiteral("ALTER") << QStringLiteral("ADD") << QStringLiteral("COLUMN")
                     << QStringLiteral("PRIMARY") << QStringLiteral("KEY") << QStringLiteral("FOREIGN")
                     << QStringLiteral("REFERENCES") << QStringLiteral("JOIN") << QStringLiteral("ON")
                     << QStringLiteral("LEFT") << QStringLiteral("RIGHT") << QStringLiteral("INNER")
                     << QStringLiteral("OUTER") << QStringLiteral("UNION") << QStringLiteral("ALL")
                     << QStringLiteral("DISTINCT") << QStringLiteral("ORDER") << QStringLiteral("BY")
                     << QStringLiteral("GROUP") << QStringLiteral("HAVING") << QStringLiteral("LIMIT")
                     << QStringLiteral("OFFSET") << QStringLiteral("AND") << QStringLiteral("OR")
                     << QStringLiteral("NOT") << QStringLiteral("NULL") << QStringLiteral("IS")
                     << QStringLiteral("IN") << QStringLiteral("LIKE") << QStringLiteral("BETWEEN")
                     << QStringLiteral("AS") << QStringLiteral("ASC") << QStringLiteral("DESC")
                     << QStringLiteral("TRUE") << QStringLiteral("FALSE") << QStringLiteral("INDEX")
                     << QStringLiteral("VIEW") << QStringLiteral("DATABASE");

        def.types << QStringLiteral("INT") << QStringLiteral("INTEGER") << QStringLiteral("VARCHAR")
                  << QStringLiteral("TEXT") << QStringLiteral("CHAR") << QStringLiteral("BOOLEAN")
                  << QStringLiteral("DATE") << QStringLiteral("DATETIME") << QStringLiteral("TIMESTAMP")
                  << QStringLiteral("FLOAT") << QStringLiteral("DOUBLE") << QStringLiteral("DECIMAL")
                  << QStringLiteral("BIGINT") << QStringLiteral("SMALLINT");

        def.singleLineComment = QStringLiteral("--");
    }

    return def;
}

// ============================================================================
// CodeHighlighter - 高亮处理
// ============================================================================

QVector<CodeHighlighter::Fragment> CodeHighlighter::highlightLine(const QString& line,
                                                                    const LanguageDef& langDef)
{
    QVector<Fragment> fragments;

    if (line.isEmpty()) {
        return fragments;
    }

    // 1. 高亮字符串（优先级最高）
    highlightStrings(fragments, line, langDef.stringColor);

    // 2. 高亮注释
    if (!langDef.singleLineComment.isEmpty()) {
        highlightComments(fragments, line, langDef.singleLineComment, langDef.commentColor);
    }

    // 3. 高亮关键词
    if (!langDef.keywords.isEmpty()) {
        highlightKeywords(fragments, line, langDef.keywords, langDef.keywordColor, true);
    }

    // 4. 高亮类型
    if (!langDef.types.isEmpty()) {
        highlightKeywords(fragments, line, langDef.types, langDef.typeColor, false);
    }

    // 5. 高亮数字
    highlightNumbers(fragments, line, langDef.numberColor);

    // 合并重叠片段
    mergeFragments(fragments);

    return fragments;
}

void CodeHighlighter::highlightStrings(QVector<Fragment>& fragments, const QString& line,
                                         const QColor& color)
{
    QRegularExpression doubleQuoteRegex(QStringLiteral("\"(?:[^\"\\\\]|\\\\.)*\""));
    QRegularExpressionMatchIterator it = doubleQuoteRegex.globalMatch(line);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        Fragment frag;
        frag.start = match.capturedStart();
        frag.length = match.capturedLength();
        frag.color = color;
        fragments.append(frag);
    }

    QRegularExpression singleQuoteRegex(QStringLiteral("'(?:[^'\\\\]|\\\\.)*'"));
    it = singleQuoteRegex.globalMatch(line);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        Fragment frag;
        frag.start = match.capturedStart();
        frag.length = match.capturedLength();
        frag.color = color;
        fragments.append(frag);
    }
}

void CodeHighlighter::highlightNumbers(QVector<Fragment>& fragments, const QString& line,
                                          const QColor& color)
{
    QRegularExpression numberRegex(QStringLiteral("\\b(0[xX][0-9a-fA-F]+|\\d+\\.?\\d*(?:[eE][+-]?\\d+)?)\\b"));
    QRegularExpressionMatchIterator it = numberRegex.globalMatch(line);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool covered = false;
        for (const Fragment& frag : fragments) {
            if (match.capturedStart() >= frag.start &&
                match.capturedStart() < frag.start + frag.length) {
                covered = true;
                break;
            }
        }
        if (!covered) {
            Fragment frag;
            frag.start = match.capturedStart();
            frag.length = match.capturedLength();
            frag.color = color;
            fragments.append(frag);
        }
    }
}

void CodeHighlighter::highlightKeywords(QVector<Fragment>& fragments, const QString& line,
                                          const QStringList& keywords, const QColor& color, bool bold)
{
    for (const QString& keyword : keywords) {
        QString pattern = QStringLiteral("\\b") + QRegularExpression::escape(keyword) + QStringLiteral("\\b");
        QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator it = regex.globalMatch(line);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            bool covered = false;
            for (const Fragment& frag : fragments) {
                if (match.capturedStart() >= frag.start &&
                    match.capturedStart() < frag.start + frag.length) {
                    covered = true;
                    break;
                }
            }
            if (!covered) {
                Fragment frag;
                frag.start = match.capturedStart();
                frag.length = match.capturedLength();
                frag.color = color;
                frag.bold = bold;
                fragments.append(frag);
            }
        }
    }
}

void CodeHighlighter::highlightComments(QVector<Fragment>& fragments, const QString& line,
                                          const QString& pattern, const QColor& color)
{
    int commentPos = line.indexOf(pattern);
    if (commentPos >= 0) {
        bool inString = false;
        for (const Fragment& frag : fragments) {
            if (commentPos >= frag.start &&
                commentPos < frag.start + frag.length) {
                inString = true;
                break;
            }
        }
        if (!inString) {
            Fragment frag;
            frag.start = commentPos;
            frag.length = line.length() - commentPos;
            frag.color = color;
            fragments.append(frag);
        }
    }
}

void CodeHighlighter::mergeFragments(QVector<Fragment>& fragments)
{
    if (fragments.size() <= 1) return;

    std::sort(fragments.begin(), fragments.end(),
              [](const Fragment& a, const Fragment& b) { return a.start < b.start; });

    QVector<Fragment> merged;
    for (const Fragment& frag : fragments) {
        bool covered = false;
        for (Fragment& existing : merged) {
            if (frag.start >= existing.start &&
                frag.start + frag.length <= existing.start + existing.length) {
                covered = true;
                break;
            }
            if (frag.start < existing.start + existing.length &&
                frag.start + frag.length > existing.start) {
                covered = true;
                break;
            }
        }
        if (!covered) {
            merged.append(frag);
        }
    }
    fragments = merged;
}

// ============================================================================
// CodeHighlighter - 绘制
// ============================================================================

void CodeHighlighter::drawCode(QPainter* painter, const QRect& rect,
                                const QString& code, const QString& language,
                                const QFont& font)
{
    // 获取设备像素比
    qreal dpr = painter->device()->devicePixelRatioF();

    // 生成缓存键（包含 DPR）
    QString cacheKey = generateCacheKey(code, language, font, rect.width(), dpr);

    // 检查缓存
    auto it = s_cache.find(cacheKey);
    if (it != s_cache.end()) {
        // 使用缓存
        painter->drawPixmap(rect.topLeft(), it.value());
        return;
    }

    // 检查缓存大小，必要时清理
    if (s_cache.size() >= s_maxCacheSize) {
        // 简单策略：清除一半缓存
        int removeCount = s_maxCacheSize / 2;
        auto it = s_cache.begin();
        while (it != s_cache.end() && removeCount > 0) {
            it = s_cache.erase(it);
            --removeCount;
        }
    }

    // 渲染到 pixmap（支持高 DPI）
    QSize pixmapSize = rect.size() * dpr;
    QPixmap pixmap(pixmapSize);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter pixmapPainter(&pixmap);
    LanguageDef langDef = getLanguageDef(language);
    drawCodeDirect(&pixmapPainter, QRect(0, 0, rect.width(), rect.height()), code, langDef, font);
    pixmapPainter.end();

    // 存入缓存
    s_cache[cacheKey] = pixmap;

    // 绘制
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void CodeHighlighter::drawCodeDirect(QPainter* painter, const QRect& rect,
                                      const QString& code, const LanguageDef& langDef,
                                      const QFont& font)
{
    painter->save();
    painter->setFont(font);
    QFontMetrics fm(font);

    QStringList lines = code.split(QLatin1Char('\n'));
    int y = rect.y() + fm.ascent();

    for (const QString& line : lines) {
        if (y > rect.bottom()) break;

        int x = rect.x();
        QVector<Fragment> fragments = highlightLine(line, langDef);

        if (fragments.isEmpty()) {
            painter->setPen(Theme::textCode);
            painter->drawText(x, y, line);
        } else {
            std::sort(fragments.begin(), fragments.end(),
                      [](const Fragment& a, const Fragment& b) { return a.start < b.start; });

            int drawnChars = 0;
            for (const Fragment& frag : fragments) {
                if (frag.start > drawnChars) {
                    painter->setPen(Theme::textCode);
                    QString normalText = line.mid(drawnChars, frag.start - drawnChars);
                    painter->drawText(x, y, normalText);
                    x += fm.horizontalAdvance(normalText);
                }

                QFont highlightFont = font;
                if (frag.bold) highlightFont.setBold(true);
                painter->setFont(highlightFont);
                painter->setPen(frag.color);

                QString highlightText = line.mid(frag.start, frag.length);
                painter->drawText(x, y, highlightText);
                x += fm.horizontalAdvance(highlightText);

                painter->setFont(font);
                drawnChars = frag.start + frag.length;
            }

            if (drawnChars < line.length()) {
                painter->setPen(Theme::textCode);
                painter->drawText(x, y, line.mid(drawnChars));
            }
        }

        y += fm.height();
    }

    painter->restore();
}

} // namespace Timeline
