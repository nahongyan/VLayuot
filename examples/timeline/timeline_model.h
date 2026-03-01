#ifndef TIMELINE_MODEL_H
#define TIMELINE_MODEL_H

/**
 * @file timeline_model.h
 * @brief 时间轴数据模型定义
 *
 * 定义了 AI 编程助手时间轴的数据模型，包括节点数据和任务管理。
 */

#include "timeline_roles.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QVariantMap>
#include <vector>

namespace Timeline {

/**
 * @struct TaskItem
 * @brief 任务项数据结构
 *
 * 存储单个任务的所有数据。
 */
struct TaskItem
{
    QString id;                   ///< 任务唯一标识
    QString name;                 ///< 任务名称
    TaskStatus status = TaskStatus::Pending;  ///< 任务状态
    int indent = 0;               ///< 缩进级别（用于显示依赖关系）
    QString dependsOn;            ///< 依赖的任务 ID
    QString description;          ///< 任务描述
};

/**
 * @struct TimelineNode
 * @brief 时间轴节点数据结构
 *
 * 存储单个时间轴节点的所有数据。
 */
struct TimelineNode
{
    QString id;                         ///< 节点唯一标识
    NodeType type = NodeType::UserMessage;  ///< 节点类型
    qint64 timestamp = 0;               ///< 时间戳（毫秒）

    // 消息内容
    QString content;                    ///< 主文本内容
    MessageSource source = MessageSource::User;  ///< 消息来源
    bool isStreaming = false;           ///< 是否正在流式输出
    bool isExpanded = true;             ///< 是否展开（用于折叠面板）

    // 代码块专用
    QString language;                   ///< 编程语言
    QString code;                       ///< 代码内容

    // 工具调用专用
    QString toolName;                   ///< 工具名称
    QVariantMap toolArgs;               ///< 工具参数
    QVariantMap toolResult;             ///< 工具结果
    ToolStatus toolStatus = ToolStatus::Pending;  ///< 工具状态

    // 思考过程专用
    QStringList thinkingSteps;          ///< 思考步骤列表

    // 任务列表专用
    std::vector<TaskItem> tasks;        ///< 任务列表
    int completedCount = 0;             ///< 已完成数量
};

/**
 * @class TimelineModel
 * @brief 时间轴数据模型
 *
 * 提供 AI 编程助手时间轴的数据，支持：
 * - 多种节点类型（消息、代码、工具、思考、任务）
 * - 流式输出更新
 * - 任务状态管理
 * - 折叠/展开状态
 *
 * ## 使用示例
 * @code
 * TimelineModel model;
 * model.addUserMessage("请帮我实现一个排序算法");
 * model.addAIMessage("好的，我来帮你实现快速排序...");
 * model.addCodeBlock("python", "def quicksort(arr):\n    ...");
 * model.addToolCall("read_file", {{"path": "src/main.py"}});
 * @endcode
 */
class TimelineModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TimelineModel(QObject* parent = nullptr);

    // ========== QAbstractListModel 接口 ==========

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // ========== 消息管理 ==========

    /**
     * @brief 添加用户消息
     * @param content 消息内容
     * @return 节点 ID
     */
    QString addUserMessage(const QString& content);

    /**
     * @brief 添加 AI 消息
     * @param content 消息内容
     * @param streaming 是否流式输出
     * @return 节点 ID
     */
    QString addAIMessage(const QString& content, bool streaming = false);

    /**
     * @brief 更新流式输出文本
     * @param nodeId 节点 ID
     * @param chunk 新增的文本片段
     */
    void appendStreamingText(const QString& nodeId, const QString& chunk);

    /**
     * @brief 完成流式输出
     * @param nodeId 节点 ID
     */
    void finishStreaming(const QString& nodeId);

    // ========== 代码块管理 ==========

    /**
     * @brief 添加代码块
     * @param language 编程语言
     * @param code 代码内容
     * @return 节点 ID
     */
    QString addCodeBlock(const QString& language, const QString& code);

    // ========== 工具调用管理 ==========

    /**
     * @brief 添加工具调用
     * @param toolName 工具名称
     * @param args 工具参数
     * @return 节点 ID
     */
    QString addToolCall(const QString& toolName, const QVariantMap& args);

    /**
     * @brief 更新工具状态
     * @param nodeId 节点 ID
     * @param status 新状态
     */
    void updateToolStatus(const QString& nodeId, ToolStatus status);

    /**
     * @brief 设置工具结果
     * @param nodeId 节点 ID
     * @param result 结果数据
     */
    void setToolResult(const QString& nodeId, const QVariantMap& result);

    // ========== 思考过程管理 ==========

    /**
     * @brief 添加思考过程
     * @param steps 思考步骤列表
     * @return 节点 ID
     */
    QString addThinking(const QStringList& steps);

    /**
     * @brief 添加思考步骤
     * @param nodeId 节点 ID
     * @param step 步骤内容
     */
    void addThinkingStep(const QString& nodeId, const QString& step);

    // ========== 任务列表管理 ==========

    /**
     * @brief 添加任务列表
     * @param tasks 任务列表
     * @return 节点 ID
     */
    QString addTaskList(const std::vector<TaskItem>& tasks);

    /**
     * @brief 更新任务状态
     * @param nodeId 节点 ID
     * @param taskId 任务 ID
     * @param status 新状态
     */
    void updateTaskStatus(const QString& nodeId, const QString& taskId, TaskStatus status);

    /**
     * @brief 添加任务
     * @param nodeId 节点 ID
     * @param task 任务项
     */
    void addTask(const QString& nodeId, const TaskItem& task);

    // ========== 折叠/展开 ==========

    /**
     * @brief 切换节点折叠状态
     * @param nodeId 节点 ID
     */
    void toggleExpanded(const QString& nodeId);

    /**
     * @brief 设置节点折叠状态
     * @param nodeId 节点 ID
     * @param expanded 是否展开
     */
    void setExpanded(const QString& nodeId, bool expanded);

    // ========== 数据管理 ==========

    /**
     * @brief 清空所有节点
     */
    void clear();

    /**
     * @brief 加载示例数据
     */
    void loadSampleData();

    /**
     * @brief 根据 ID 获取节点索引
     * @param nodeId 节点 ID
     * @return 模型索引，未找到返回无效索引
     */
    QModelIndex indexForNodeId(const QString& nodeId) const;

private:
    QString generateId() const;
    void updateCompletedCount(TimelineNode& node);

    // ========== 成员变量 ==========

    std::vector<TimelineNode> m_nodes;  ///< 节点数据
};

} // namespace Timeline

#endif // TIMELINE_MODEL_H
