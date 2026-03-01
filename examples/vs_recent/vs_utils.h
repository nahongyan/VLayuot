#ifndef VS_UTILS_H
#define VS_UTILS_H

/**
 * @file vs_utils.h
 * @brief 工具函数声明
 *
 * 提供图标绘制、日期格式化等工具函数。
 */

#include <QColor>
#include <QDateTime>
#include <QRect>

class QPainter;

namespace VS {
namespace Utils {

/**
 * @brief 绘制解决方案图标
 *
 * 绘制带圆角矩形背景和字母/符号缩写的图标。
 *
 * @param painter 绘制器
 * @param area 图标区域
 * @param name 项目名称（用于确定符号）
 * @param bgColor 背景颜色
 */
void drawSolutionIcon(QPainter* painter, const QRect& area,
                      const QString& name, const QColor& bgColor);

/**
 * @brief 绘制图钉图标
 *
 * 绘制固定/取消固定的图钉图标。
 *
 * @param painter 绘制器
 * @param area 图标区域
 * @param pinned 是否已固定
 * @param hovered 是否悬停
 */
void drawPinIcon(QPainter* painter, const QRect& area,
                 bool pinned, bool hovered);

/**
 * @brief 格式化日期时间
 *
 * 将日期时间格式化为友好的显示格式：
 * - 今天：显示时间 (h:mm)
 * - 昨天：显示"昨天"
 * - 本月：显示"X月X日"
 * - 其他：显示完整日期 (yyyy/M/d h:mm)
 *
 * @param dt 日期时间
 * @return 格式化后的字符串
 */
QString formatDate(const QDateTime& dt);

} // namespace Utils
} // namespace VS

#endif // VS_UTILS_H
