#include "vs_utils.h"
#include "vs_theme.h"

#include <QPainter>
#include <QDate>
#include <QFont>

namespace VS {
namespace Utils {

void drawSolutionIcon(QPainter* painter, const QRect& area,
                      const QString& name, const QColor& bgColor)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 计算图标尺寸（留 6px 边距）
    int s = qMin(area.width(), area.height()) - 6;
    QRect sq(area.center().x() - s / 2, area.center().y() - s / 2, s, s);

    // 绘制圆角背景
    painter->setBrush(bgColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(sq, 4, 4);

    // 绘制符号
    painter->setPen(Qt::white);
    QFont font;
    font.setFamily("Segoe UI");
    font.setPixelSize(s > 26 ? 14 : 10);
    font.setBold(true);
    painter->setFont(font);

    // 根据文件类型选择符号
    QString symbol;
    if (name.endsWith(".sln", Qt::CaseInsensitive)) {
        symbol = QString(QChar(0x2756));  // ❖
    } else {
        symbol = name.left(1).toUpper();
    }

    painter->drawText(sq, Qt::AlignCenter, symbol);
    painter->restore();
}

void drawPinIcon(QPainter* painter, const QRect& area,
                 bool pinned, bool hovered)
{
    // 未固定且未悬停时不绘制
    if (!pinned && !hovered) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 选择颜色
    QColor color = pinned ? Theme::pinActive : Theme::pinColor;
    painter->setPen(QPen(color, 1.5));
    painter->setBrush(Qt::NoBrush);

    int cx = area.center().x();
    int cy = area.center().y();
    int sz = 5;

    if (pinned) {
        // 实心圆点 + 竖线
        painter->setBrush(color);
        painter->drawEllipse(QPoint(cx, cy - 1), sz, sz);
        painter->drawLine(cx, cy + sz - 1, cx, cy + sz + 3);
    } else {
        // 空心圆点 + 竖线
        painter->drawEllipse(QPoint(cx, cy - 1), sz, sz);
        painter->drawLine(cx, cy + sz - 1, cx, cy + sz + 3);
    }

    painter->restore();
}

QString formatDate(const QDateTime& dt)
{
    if (!dt.isValid()) {
        return {};
    }

    QDate today = QDate::currentDate();
    QDate date = dt.date();

    if (date == today) {
        // 今天：显示时间
        return dt.toString("h:mm");
    }

    if (date == today.addDays(-1)) {
        // 昨天
        return QStringLiteral("昨天");
    }

    if (date.year() == today.year() && date.month() == today.month()) {
        // 本月：显示"X月X日"
        return dt.toString("M月d日");
    }

    // 其他：显示完整日期时间
    return dt.toString("yyyy/M/d h:mm");
}

} // namespace Utils
} // namespace VS
