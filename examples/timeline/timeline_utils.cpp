#include "timeline_utils.h"
#include "qjsonobject.h"
#include "timeline_theme.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QTextDocument>
#include <QJsonDocument>
#include <QPolygonF>
#include <QDateTime>

namespace Timeline {
namespace Utils {

// ============================================================================
// 图标绘制
// ============================================================================

void drawToolIcon(QPainter* painter, const QRect& area,
                  const QString& toolName, ToolStatus status)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 获取图标类型对应的颜色
    QString iconType = getToolIconType(toolName);
    QColor bgColor;
    if (iconType == QStringLiteral("file")) {
        bgColor = Theme::iconFile;
    } else if (iconType == QStringLiteral("command")) {
        bgColor = Theme::iconCommand;
    } else if (iconType == QStringLiteral("network")) {
        bgColor = Theme::iconNetwork;
    } else if (iconType == QStringLiteral("edit")) {
        bgColor = Theme::iconEdit;
    } else {
        bgColor = Theme::textSecond;
    }

    // 如果是错误状态，使用错误色
    if (status == ToolStatus::Error) {
        bgColor = Theme::error;
    }

    // 绘制圆角矩形背景
    int s = qMin(area.width(), area.height()) - 4;
    QRect sq(area.center().x() - s / 2, area.center().y() - s / 2, s, s);

    painter->setBrush(bgColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(sq, 3, 3);

    // 绘制图标符号
    painter->setPen(Qt::white);
    QFont font(QStringLiteral("Segoe UI"), s > 16 ? 10 : 8);
    font.setBold(true);
    painter->setFont(font);

    QString symbol;
    if (iconType == QStringLiteral("file")) {
        symbol = QStringLiteral("F");
    } else if (iconType == QStringLiteral("command")) {
        symbol = QStringLiteral("$");
    } else if (iconType == QStringLiteral("network")) {
        symbol = QStringLiteral("N");
    } else if (iconType == QStringLiteral("edit")) {
        symbol = QStringLiteral("E");
    } else {
        symbol = QStringLiteral("T");
    }

    painter->drawText(sq, Qt::AlignCenter, symbol);
    painter->restore();
}

void drawStatusIndicator(QPainter* painter, const QRect& area,
                         ToolStatus status)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QColor color;
    switch (status) {
    case ToolStatus::Pending:
        color = Theme::textSecond;
        break;
    case ToolStatus::Running:
        color = Theme::running;
        break;
    case ToolStatus::Success:
        color = Theme::success;
        break;
    case ToolStatus::Error:
        color = Theme::error;
        break;
    default:
        color = Theme::textSecond;
        break;
    }

    int r = qMin(area.width(), area.height()) / 2 - 1;
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(area.center(), r, r);

    painter->restore();
}

void drawExpandArrow(QPainter* painter, const QRect& area, bool expanded)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int cx = area.center().x();
    int cy = area.center().y();
    int sz = 4;

    painter->setBrush(Theme::textSecond);
    painter->setPen(Qt::NoPen);

    QPolygonF arrow;
    if (expanded) {
        // 向下箭头 v
        arrow << QPointF(cx - sz, cy - sz + 2)
              << QPointF(cx + sz, cy - sz + 2)
              << QPointF(cx, cy + sz - 2);
    } else {
        // 向右箭头 >
        arrow << QPointF(cx - sz + 2, cy - sz)
              << QPointF(cx + sz - 2, cy)
              << QPointF(cx - sz + 2, cy + sz);
    }

    painter->drawPolygon(arrow);
    painter->restore();
}

void drawCopyButton(QPainter* painter, const QRect& area,
                    bool hovered, bool copied)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QColor color = hovered ? Theme::textPrimary : Theme::textSecond;
    if (copied) {
        color = Theme::success;
    }

    painter->setPen(QPen(color, 1.5));
    painter->setBrush(Qt::NoBrush);

    int cx = area.center().x();
    int cy = area.center().y();
    int sz = 6;

    if (copied) {
        // 绘制勾号
        QPolygonF check;
        check << QPointF(cx - sz + 1, cy)
              << QPointF(cx - 2, cy + sz - 2)
              << QPointF(cx + sz, cy - sz + 2);
        painter->drawPolyline(check);
    } else {
        // 绘制复制图标（两个叠加的矩形）
        QRect r1(cx - sz, cy - sz + 2, sz * 2 - 2, sz * 2 - 2);
        QRect r2(cx - sz + 3, cy - sz - 1, sz * 2 - 2, sz * 2 - 2);

        painter->drawRect(r2);
        painter->drawLine(r2.bottomLeft(), r1.topLeft());
        painter->drawLine(r2.bottomRight(), r1.topRight());
    }

    painter->restore();
}

// ============================================================================
// 文本处理
// ============================================================================

int calculateTextHeight(const QString& text, int width, const QFont& font)
{
    if (text.isEmpty() || width <= 0) {
        return 0;
    }

    QFontMetrics fm(font);
    QRect boundingRect = fm.boundingRect(QRect(0, 0, width, 10000),
                                          Qt::TextWordWrap, text);
    return boundingRect.height();
}

int calculateCodeHeight(const QString& code, int width, const QFont& font)
{
    if (code.isEmpty() || width <= 0) {
        return 0;
    }

    // 代码块需要额外的行号区域和边距
    int lineCount = code.count(QLatin1Char('\n')) + 1;
    QFontMetrics fm(font);
    int lineHeight = fm.height();
    int padding = 16;  // 上下内边距

    return lineCount * lineHeight + padding;
}

QString formatTimestamp(qint64 timestamp)
{
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
    QDateTime now = QDateTime::currentDateTime();
    qint64 secsAgo = dt.secsTo(now);

    if (secsAgo < 60) {
        return QStringLiteral("刚刚");
    } else if (secsAgo < 3600) {
        int mins = secsAgo / 60;
        return QStringLiteral("%1分钟前").arg(mins);
    } else if (secsAgo < 86400) {
        int hours = secsAgo / 3600;
        return QStringLiteral("%1小时前").arg(hours);
    } else if (secsAgo < 604800) {
        int days = secsAgo / 86400;
        return QStringLiteral("%1天前").arg(days);
    } else {
        return dt.toString(QStringLiteral("M/d h:mm"));
    }
}

QString formatToolArgs(const QVariantMap& args)
{
    if (args.isEmpty()) {
        return QStringLiteral("{}");
    }

    QJsonDocument doc(QJsonObject::fromVariantMap(args));
    QString json = doc.toJson(QJsonDocument::Indented);

    // 限制显示长度
    if (json.length() > 500) {
        json = json.left(500) + QStringLiteral("...");
    }

    return json;
}

QString truncateText(const QString& text, int maxChars)
{
    if (text.length() <= maxChars) {
        return text;
    }
    return text.left(maxChars - 3) + QStringLiteral("...");
}

// ============================================================================
// 工具类型识别
// ============================================================================

QString getToolIconType(const QString& toolName)
{
    QString name = toolName.toLower();

    if (name.contains(QStringLiteral("read")) ||
        name.contains(QStringLiteral("write")) ||
        name.contains(QStringLiteral("file")) ||
        name.contains(QStringLiteral("glob")) ||
        name.contains(QStringLiteral("grep"))) {
        return QStringLiteral("file");
    }

    if (name.contains(QStringLiteral("bash")) ||
        name.contains(QStringLiteral("shell")) ||
        name.contains(QStringLiteral("exec")) ||
        name.contains(QStringLiteral("command")) ||
        name.contains(QStringLiteral("run"))) {
        return QStringLiteral("command");
    }

    if (name.contains(QStringLiteral("http")) ||
        name.contains(QStringLiteral("fetch")) ||
        name.contains(QStringLiteral("request")) ||
        name.contains(QStringLiteral("web"))) {
        return QStringLiteral("network");
    }

    if (name.contains(QStringLiteral("edit")) ||
        name.contains(QStringLiteral("modify")) ||
        name.contains(QStringLiteral("insert")) ||
        name.contains(QStringLiteral("delete"))) {
        return QStringLiteral("edit");
    }

    return QStringLiteral("default");
}

QString getToolDisplayName(const QString& toolName)
{
    // 将下划线/驼峰转换为空格分隔的显示名称
    QString displayName;

    for (int i = 0; i < toolName.length(); ++i) {
        QChar c = toolName[i];
        if (c.isUpper() && i > 0) {
            displayName += QLatin1Char(' ');
        } else if (c == QLatin1Char('_') || c == QLatin1Char('-')) {
            displayName += QLatin1Char(' ');
            continue;
        }
        displayName += c;
    }

    // 首字母大写
    if (!displayName.isEmpty()) {
        displayName[0] = displayName[0].toUpper();
    }

    return displayName;
}

} // namespace Utils
} // namespace Timeline
