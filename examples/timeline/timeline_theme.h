#ifndef TIMELINE_THEME_H
#define TIMELINE_THEME_H

/**
 * @file timeline_theme.h
 * @brief 时间轴主题颜色定义 (VSCode Claude Code 风格)
 *
 * 采用深色扁平化设计，参考 VSCode Claude Code 插件的视觉风格。
 */

#include <QColor>
#include <QFont>
#include <QFontDatabase>

namespace Timeline {
namespace Theme {

// ============================================================================
// 背景色 - 深色扁平化（调整亮度）
// ============================================================================

/// 主背景色 #1e1e1e (VSCode 默认深色)
constexpr QColor bgBase      {30,  30,  30};

/// 表面背景色 #252526
constexpr QColor bgSurface   {37,  37,  38};

/// 代码块背景 #1e1e1e
constexpr QColor bgCodeBlock {30,  30,  30};

/// 工具卡片背景 #252526
constexpr QColor bgToolCard  {37,  37,  38};

/// 思考过程背景 #252526
constexpr QColor bgThinking  {37,  37,  38};

/// 输入区域背景 #252526
constexpr QColor bgInput     {37,  37,  38};

// ============================================================================
// 文本色
// ============================================================================

/// 主要文本色 #e0e0e0
constexpr QColor textPrimary {224, 224, 224};

/// 次要文本色 #888888
constexpr QColor textSecond  {136, 136, 136};

/// 代码文本色 #d4d4d4
constexpr QColor textCode    {212, 212, 212};

/// 链接文本色 #3794ff
constexpr QColor textLink    {55, 148, 255};

// ============================================================================
// 时间线指示器颜色 (按操作类型)
// ============================================================================

/// 时间线轨道颜色 #3c3c3c (更亮，能看清)
constexpr QColor timelineTrack  {60,  60,  60};

/// 思考 - 白色
constexpr QColor dotThinking   {255, 255, 255};

/// 命令执行 - 橙色
constexpr QColor dotCommand    {255, 140, 50};

/// 文件操作 - 绿色
constexpr QColor dotFile       {80,  200, 120};

/// 用户消息 - 蓝色
constexpr QColor dotUser       {86,  156, 214};

/// AI 消息 - 橙棕色 (Claude 风格)
constexpr QColor dotAI         {217, 119, 87};

/// 默认/未知 - 灰色
constexpr QColor dotDefault    {100, 100, 100};

// ============================================================================
// 强调色
// ============================================================================

/// AI 强调色（Claude 风格橙棕色）#d97757
constexpr QColor accentAI    {217, 119, 87};

/// 用户强调色（蓝色）#569cd6
constexpr QColor accentUser  {86,  156, 214};

/// 成功色 #4caf50
constexpr QColor success     {76,  175, 80};

/// 警告色 #ff9800
constexpr QColor warning     {255, 152, 0};

/// 错误色 #f44336
constexpr QColor error       {244, 67,  54};

/// 运行中色 #569cd6
constexpr QColor running     {86,  156, 214};

// ============================================================================
// 语法高亮色
// ============================================================================

constexpr QColor syntaxKeyword{86,  156, 214};
constexpr QColor syntaxString {206, 145, 120};
constexpr QColor syntaxComment{106, 153, 85};
constexpr QColor syntaxNumber {181, 206, 168};
constexpr QColor syntaxFunction{220, 220, 170};
constexpr QColor syntaxType   {78,  201, 176};

// ============================================================================
// 图标色 (工具卡片使用)
// ============================================================================

/// 工具图标色（文件操作）#519aba
constexpr QColor iconFile    {81, 154, 186};

/// 工具图标色（命令执行）#89d185
constexpr QColor iconCommand {137, 209, 133};

/// 工具图标色（网络请求）#6d9dce
constexpr QColor iconNetwork {109, 157, 206};

/// 工具图标色（代码编辑）#dcdcaa
constexpr QColor iconEdit    {220, 220, 170};

// ============================================================================
// 边框色 - 低调但可见
// ============================================================================

/// 分隔线颜色 #3c3c3c
constexpr QColor separator   {60,  60,  60};

/// 代码块边框 #3c3c3c
constexpr QColor codeBorder  {60,  60,  60};

/// 工具卡片边框 #3c3c3c
constexpr QColor toolBorder  {60,  60,  60};

// ============================================================================
// 滚动条色
// ============================================================================

constexpr QColor scrollBg    {20,  20,  20};
constexpr QColor scrollHandle{60,  60,  60};

// ============================================================================
// 布局尺寸常量
// ============================================================================

/// 时间线 X 坐标（距离左边）
constexpr int timelineX = 16;

/// 时间线圆点半径 (12px 直径)
constexpr int timelineDotRadius = 6;

/// 内容左边距（时间线右侧）
constexpr int contentLeftMargin = 40;

/// 内容右边距
constexpr int contentRightMargin = 32;

/// 内容上下边距
constexpr int contentVMargin = 12;

/// 水平边距总和（用于宽度计算）
constexpr int totalHMargin = contentLeftMargin + contentRightMargin;

/// 滚动条宽度
constexpr int scrollBarWidth = 12;

/// 组件圆角半径
constexpr int borderRadius = 6;

// ============================================================================
// 辅助函数 - 获取节点类型对应的时间线圆点颜色
// ============================================================================

inline QColor getTimelineDotColor(int nodeType, const QString& toolName = QString()) {
    // 根据 nodeType 判断
    // 0=UserMessage, 1=AIMessage, 2=CodeBlock, 3=ToolCall, 4=ThinkingStep, 5=TaskList
    switch (nodeType) {
    case 0:  // UserMessage
        return dotUser;
    case 1:  // AIMessage
        return dotAI;
    case 2:  // CodeBlock
        return dotFile;
    case 3:  // ToolCall - 根据工具名称细分
        if (toolName.contains(QStringLiteral("read"), Qt::CaseInsensitive) ||
            toolName.contains(QStringLiteral("file"), Qt::CaseInsensitive)) {
            return dotFile;
        }
        if (toolName.contains(QStringLiteral("bash"), Qt::CaseInsensitive) ||
            toolName.contains(QStringLiteral("command"), Qt::CaseInsensitive) ||
            toolName.contains(QStringLiteral("exec"), Qt::CaseInsensitive)) {
            return dotCommand;
        }
        return dotCommand;  // 默认命令色
    case 4:  // ThinkingStep
        return dotThinking;
    case 5:  // TaskList
        return dotAI;
    default:
        return dotDefault;
    }
}

// ============================================================================
// 字体设置 - 支持中文
// ============================================================================

/// 获取默认文本字体（支持中文）
inline QFont textFont(int pointSize = 10) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSize(pointSize);
    return font;
}

/// 获取代码字体
inline QFont codeFont(int pointSize = 10) {
    QFont font(QStringLiteral("Consolas"), pointSize);
    if (!font.exactMatch()) {
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        font.setPointSize(pointSize);
    }
    return font;
}

/// 获取标题字体
inline QFont titleFont(int pointSize = 10) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSize(pointSize);
    font.setBold(true);
    return font;
}

} // namespace Theme
} // namespace Timeline

#endif // TIMELINE_THEME_H
