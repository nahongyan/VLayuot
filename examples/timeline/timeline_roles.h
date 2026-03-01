#ifndef TIMELINE_ROLES_H
#define TIMELINE_ROLES_H

/**
 * @file timeline_roles.h
 * @brief 时间轴数据角色定义
 *
 * 定义了 AI 编程助手时间轴组件使用的自定义 Qt ItemDataRole。
 */

#include <Qt>

namespace Timeline {

/**
 * @enum NodeType
 * @brief 时间轴节点类型
 *
 * 定义时间轴上不同类型的显示节点。
 */
enum class NodeType {
    UserMessage,      ///< 用户消息气泡
    AIMessage,        ///< AI 回复消息（支持流式）
    CodeBlock,        ///< 代码块
    ToolCall,         ///< 工具调用卡片
    ThinkingStep,     ///< 思考过程（可折叠）
    TaskList          ///< 任务列表
};

/**
 * @enum TaskStatus
 * @brief 任务状态
 */
enum class TaskStatus {
    Pending,      ///< 待处理
    InProgress,   ///< 进行中
    Completed,    ///< 已完成
    Failed,       ///< 失败
    Skipped       ///< 已跳过
};

/**
 * @enum ToolStatus
 * @brief 工具调用状态
 */
enum class ToolStatus {
    Pending,    ///< 等待执行
    Running,    ///< 执行中
    Success,    ///< 执行成功
    Error       ///< 执行失败
};

/**
 * @enum Roles
 * @brief 自定义数据角色
 *
 * 用于在 Model 中存储和检索时间轴节点数据。
 */
enum Roles {
    // 基础信息
    NodeIdRole      = Qt::UserRole + 100,  ///< 节点唯一标识
    NodeTypeRole,                          ///< 节点类型 (NodeType)
    TimestampRole,                         ///< 时间戳

    // 消息内容
    ContentRole,                           ///< 主文本内容
    IsStreamingRole,                       ///< 是否正在流式输出
    IsExpandedRole,                        ///< 是否展开（用于折叠面板）

    // 代码块专用
    LanguageRole,                          ///< 编程语言
    CodeRole,                              ///< 代码内容

    // 工具调用专用
    ToolNameRole,                          ///< 工具名称
    ToolArgsRole,                          ///< 工具参数 (QVariantMap)
    ToolResultRole,                        ///< 工具结果 (QVariantMap)
    ToolStatusRole,                        ///< 工具状态 (ToolStatus)

    // 思考过程专用
    ThinkingStepsRole,                     ///< 思考步骤列表 (QStringList)

    // 任务列表专用
    TasksRole,                             ///< 任务列表 (QVariantList)
    TaskCountRole,                         ///< 任务总数
    CompletedCountRole,                    ///< 已完成数量
    ProgressRole,                          ///< 进度百分比 (0-100)

    // 角色标识
    MessageRole,                           ///< 消息角色 (User/Assistant/System)
};

/**
 * @enum MessageSource
 * @brief 消息来源
 */
enum class MessageSource {
    User,       ///< 用户消息
    Assistant,  ///< AI 助手消息
    System      ///< 系统消息
};

} // namespace Timeline

#endif // TIMELINE_ROLES_H
