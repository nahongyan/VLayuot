#ifndef TIMELINE_UTILS_H
#define TIMELINE_UTILS_H

/**
 * @file timeline_utils.h
 * @brief 时间轴工具函数声明
 *
 * 提供图标绘制、文本处理、语法高亮等工具函数。
 */

#include "timeline_roles.h"

#include <QColor>
#include <QRect>
#include <QString>
#include <QVariantMap>

class QPainter;
class QTextDocument;

namespace Timeline {
namespace Utils {

// ============================================================================
// 图标绘制
// ============================================================================

/**
 * @brief 绘制工具图标
 *
 * 根据工具类型绘制对应的图标。
 *
 * @param painter 绘制器
 * @param area 图标区域
 * @param toolName 工具名称
 * @param status 工具状态
 */
void drawToolIcon(QPainter* painter, const QRect& area,
                  const QString& toolName, ToolStatus status);

/**
 * @brief 绘制状态指示器
 *
 * 绘制工具执行状态的小圆点。
 *
 * @param painter 绘制器
 * @param area 指示器区域
 * @param status 工具状态
 */
void drawStatusIndicator(QPainter* painter, const QRect& area,
                         ToolStatus status);

/**
 * @brief 绘制展开/折叠箭头
 *
 * @param painter 绘制器
 * @param area 箭头区域
 * @param expanded 是否展开
 */
void drawExpandArrow(QPainter* painter, const QRect& area, bool expanded);

/**
 * @brief 绘制复制按钮
 *
 * @param painter 绘制器
 * @param area 按钮区域
 * @param hovered 是否悬停
 * @param copied 是否已复制
 */
void drawCopyButton(QPainter* painter, const QRect& area,
                    bool hovered, bool copied);

// ============================================================================
// 文本处理
// ============================================================================

/**
 * @brief 计算文本高度
 *
 * 计算给定宽度下文本所需的高度。
 *
 * @param text 文本内容
 * @param width 可用宽度
 * @param font 字体
 * @return 文本高度
 */
int calculateTextHeight(const QString& text, int width, const QFont& font);

/**
 * @brief 计算代码块高度
 *
 * 计算代码块显示所需的高度。
 *
 * @param code 代码内容
 * @param width 可用宽度
 * @param font 字体
 * @return 代码块高度
 */
int calculateCodeHeight(const QString& code, int width, const QFont& font);

/**
 * @brief 格式化时间戳
 *
 * 将时间戳格式化为友好的显示格式。
 *
 * @param timestamp 时间戳（毫秒）
 * @return 格式化后的字符串
 */
QString formatTimestamp(qint64 timestamp);

/**
 * @brief 格式化工具参数
 *
 * 将工具参数格式化为 JSON 字符串显示。
 *
 * @param args 参数对象
 * @return 格式化后的字符串
 */
QString formatToolArgs(const QVariantMap& args);

/**
 * @brief 截断文本
 *
 * 将过长的文本截断并添加省略号。
 *
 * @param text 原始文本
 * @param maxChars 最大字符数
 * @return 截断后的文本
 */
QString truncateText(const QString& text, int maxChars);

// ============================================================================
// 工具类型识别
// ============================================================================

/**
 * @brief 获取工具图标类型
 *
 * 根据工具名称返回对应的图标类型。
 *
 * @param toolName 工具名称
 * @return 图标类型: "file" | "command" | "network" | "edit" | "default"
 */
QString getToolIconType(const QString& toolName);

/**
 * @brief 获取工具显示名称
 *
 * 将工具内部名称转换为用户友好的显示名称。
 *
 * @param toolName 工具内部名称
 * @return 显示名称
 */
QString getToolDisplayName(const QString& toolName);

} // namespace Utils
} // namespace Timeline

#endif // TIMELINE_UTILS_H
