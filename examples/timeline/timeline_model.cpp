#include "timeline_model.h"
#include "qrandom.h"

#include <QUuid>
#include <QDateTime>

namespace Timeline {

// ============================================================================
// TimelineModel
// ============================================================================

TimelineModel::TimelineModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_nodes.size());
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_nodes.size())) {
        return QVariant();
    }

    const TimelineNode& node = m_nodes[index.row()];

    switch (role) {
    // 基础信息
    case NodeIdRole:
        return node.id;
    case NodeTypeRole:
        return static_cast<int>(node.type);
    case TimestampRole:
        return node.timestamp;

    // 消息内容
    case ContentRole:
        return node.content;
    case IsStreamingRole:
        return node.isStreaming;
    case IsExpandedRole:
        return node.isExpanded;
    case MessageRole:
        return static_cast<int>(node.source);

    // 代码块专用
    case LanguageRole:
        return node.language;
    case CodeRole:
        return node.code;

    // 工具调用专用
    case ToolNameRole:
        return node.toolName;
    case ToolArgsRole:
        return node.toolArgs;
    case ToolResultRole:
        return node.toolResult;
    case ToolStatusRole:
        return static_cast<int>(node.toolStatus);

    // 思考过程专用
    case ThinkingStepsRole:
        return node.thinkingSteps;
    case ThinkingStateRole:
        return node.isThinking;

    // 任务列表专用
    case TasksRole: {
        QVariantList tasks;
        for (const auto& task : node.tasks) {
            QVariantMap taskMap;
            taskMap[QStringLiteral("id")] = task.id;
            taskMap[QStringLiteral("name")] = task.name;
            taskMap[QStringLiteral("status")] = static_cast<int>(task.status);
            taskMap[QStringLiteral("indent")] = task.indent;
            taskMap[QStringLiteral("dependsOn")] = task.dependsOn;
            taskMap[QStringLiteral("description")] = task.description;
            tasks.append(taskMap);
        }
        return tasks;
    }
    case TaskCountRole:
        return static_cast<int>(node.tasks.size());
    case CompletedCountRole:
        return node.completedCount;
    case ProgressRole:
        if (node.tasks.empty()) return 0;
        return node.completedCount * 100 / static_cast<int>(node.tasks.size());

    default:
        return QVariant();
    }
}

bool TimelineModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_nodes.size())) {
        return false;
    }

    TimelineNode& node = m_nodes[index.row()];

    switch (role) {
    case IsExpandedRole:
        node.isExpanded = value.toBool();
        emit dataChanged(index, index, {IsExpandedRole});
        return true;

    case ContentRole:
        node.content = value.toString();
        emit dataChanged(index, index, {ContentRole});
        return true;

    case IsStreamingRole:
        node.isStreaming = value.toBool();
        emit dataChanged(index, index, {IsStreamingRole});
        return true;

    case ToolStatusRole:
        node.toolStatus = static_cast<ToolStatus>(value.toInt());
        emit dataChanged(index, index, {ToolStatusRole});
        return true;

    case ToolResultRole:
        node.toolResult = value.toMap();
        emit dataChanged(index, index, {ToolResultRole});
        return true;

    case ThinkingStepsRole:
        node.thinkingSteps = value.toStringList();
        emit dataChanged(index, index, {ThinkingStepsRole});
        return true;

    case ThinkingStateRole:
        node.isThinking = value.toBool();
        emit dataChanged(index, index, {ThinkingStateRole});
        return true;

    default:
        return false;
    }
}

Qt::ItemFlags TimelineModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);

    if (index.isValid()) {
        // 所有项目都可选中
        f |= Qt::ItemIsSelectable;
    }

    return f;
}

// ============================================================================
// 消息管理
// ============================================================================

QString TimelineModel::addUserMessage(const QString& content)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::UserMessage;
    node.content = content;
    node.source = MessageSource::User;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

QString TimelineModel::addAIMessage(const QString& content, bool streaming)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::AIMessage;
    node.content = content;
    node.source = MessageSource::Assistant;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();
    node.isStreaming = streaming;

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

void TimelineModel::updateLastAIMessage(const QString& content, bool streaming)
{
    // 从后向前查找最后一条 AI 消息
    for (int i = static_cast<int>(m_nodes.size()) - 1; i >= 0; --i) {
        if (m_nodes[i].type == NodeType::AIMessage) {
            m_nodes[i].content = content;
            m_nodes[i].isStreaming = streaming;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {ContentRole, IsStreamingRole});
            return;
        }
    }

    // 如果没有找到 AI 消息，创建一条新的
    addAIMessage(content, streaming);
}

void TimelineModel::appendStreamingText(const QString& nodeId, const QString& chunk)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    node.content += chunk;

    emit dataChanged(idx, idx, {ContentRole});
}

void TimelineModel::finishStreaming(const QString& nodeId)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    node.isStreaming = false;

    emit dataChanged(idx, idx, {IsStreamingRole, ContentRole});
}

// ============================================================================
// 代码块管理
// ============================================================================

QString TimelineModel::addCodeBlock(const QString& language, const QString& code)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::CodeBlock;
    node.language = language;
    node.code = code;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

// ============================================================================
// 工具调用管理
// ============================================================================

QString TimelineModel::addToolCall(const QString& toolName, const QVariantMap& args)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::ToolCall;
    node.toolName = toolName;
    node.toolArgs = args;
    node.toolStatus = ToolStatus::Pending;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

void TimelineModel::updateToolStatus(const QString& nodeId, ToolStatus status)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    setData(idx, static_cast<int>(status), ToolStatusRole);
}

void TimelineModel::setToolResult(const QString& nodeId, const QVariantMap& result)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    setData(idx, result, ToolResultRole);
}

// ============================================================================
// 思考过程管理
// ============================================================================

QString TimelineModel::addThinking(const QStringList& steps)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::ThinkingStep;
    node.thinkingSteps = steps;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

void TimelineModel::addThinkingStep(const QString& nodeId, const QString& step)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    node.thinkingSteps.append(step);

    emit dataChanged(idx, idx, {ThinkingStepsRole});
}

void TimelineModel::updateThinkingContent(const QString& nodeId, const QString& content)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    // 将思考内容作为单个步骤存储
    if (node.thinkingSteps.isEmpty()) {
        node.thinkingSteps.append(content);
    } else {
        node.thinkingSteps[0] = content;
    }

    // 发出数据变化信号，包含 SizeHintRole 以触发视图重新计算高度
    emit dataChanged(idx, idx, {ThinkingStepsRole, Qt::SizeHintRole});
}

void TimelineModel::setThinkingState(const QString& nodeId, bool thinking)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    node.isThinking = thinking;

    emit dataChanged(idx, idx, {ThinkingStateRole});
}

// ============================================================================
// 任务列表管理
// ============================================================================

QString TimelineModel::addTaskList(const std::vector<TaskItem>& tasks)
{
    TimelineNode node;
    node.id = generateId();
    node.type = NodeType::TaskList;
    node.tasks = tasks;
    node.timestamp = QDateTime::currentMSecsSinceEpoch();
    updateCompletedCount(node);

    beginInsertRows(QModelIndex(), static_cast<int>(m_nodes.size()),
                    static_cast<int>(m_nodes.size()));
    m_nodes.push_back(node);
    endInsertRows();

    return node.id;
}

void TimelineModel::updateTaskStatus(const QString& nodeId, const QString& taskId,
                                      TaskStatus status)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];

    for (auto& task : node.tasks) {
        if (task.id == taskId) {
            task.status = status;
            break;
        }
    }

    updateCompletedCount(node);
    emit dataChanged(idx, idx, {TasksRole, CompletedCountRole, ProgressRole});
}

void TimelineModel::addTask(const QString& nodeId, const TaskItem& task)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    TimelineNode& node = m_nodes[idx.row()];
    node.tasks.push_back(task);

    emit dataChanged(idx, idx, {TasksRole, TaskCountRole});
}

// ============================================================================
// 折叠/展开
// ============================================================================

void TimelineModel::toggleExpanded(const QString& nodeId)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    bool expanded = data(idx, IsExpandedRole).toBool();
    setData(idx, !expanded, IsExpandedRole);
}

void TimelineModel::setExpanded(const QString& nodeId, bool expanded)
{
    QModelIndex idx = indexForNodeId(nodeId);
    if (!idx.isValid()) return;

    setData(idx, expanded, IsExpandedRole);
}

// ============================================================================
// 数据管理
// ============================================================================

void TimelineModel::clear()
{
    beginResetModel();
    m_nodes.clear();
    endResetModel();
}

void TimelineModel::loadSampleData()
{
    beginResetModel();
    m_nodes.clear();

    // ==================== 对话 1: 项目初始化 ====================

    // 1. 用户消息
    TimelineNode n1;
    n1.id = generateId();
    n1.type = NodeType::UserMessage;
    n1.source = MessageSource::User;
    n1.content = QStringLiteral("你好，我想创建一个 Qt 项目，用于展示任务管理界面");
    n1.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n1);

    // 2. 思考过程
    TimelineNode n2;
    n2.id = generateId();
    n2.type = NodeType::ThinkingStep;
    n2.thinkingSteps << QStringLiteral("分析用户需求：Qt 任务管理界面")
                     << QStringLiteral("确定技术栈：Qt Widgets + C++")
                     << QStringLiteral("规划项目结构");
    n2.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n2);

    // 3. AI 消息
    TimelineNode n3;
    n3.id = generateId();
    n3.type = NodeType::AIMessage;
    n3.source = MessageSource::Assistant;
    n3.content = QStringLiteral("好的！我来帮你创建一个 Qt 任务管理界面项目。首先我们需要规划一下项目结构。");
    n3.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n3);

    // 4. 任务列表
    TimelineNode n4;
    n4.id = generateId();
    n4.type = NodeType::TaskList;
    n4.tasks = {
        {generateId(), QStringLiteral("创建项目结构"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("编写 CMakeLists.txt"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("实现主窗口类"), TaskStatus::InProgress, 0},
        {generateId(), QStringLiteral("添加任务列表视图"), TaskStatus::Pending, 0},
        {generateId(), QStringLiteral("实现任务添加功能"), TaskStatus::Pending, 0},
        {generateId(), QStringLiteral("添加任务编辑功能"), TaskStatus::Pending, 0}
    };
    n4.completedCount = 2;
    n4.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n4);

    // 5. 工具调用 - 创建文件
    TimelineNode n5;
    n5.id = generateId();
    n5.type = NodeType::ToolCall;
    n5.toolName = QStringLiteral("write_file");
    n5.toolArgs = {{QStringLiteral("path"), QStringLiteral("CMakeLists.txt")}};
    n5.toolStatus = ToolStatus::Success;
    n5.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n5);

    // 6. AI 消息
    TimelineNode n6;
    n6.id = generateId();
    n6.type = NodeType::AIMessage;
    n6.source = MessageSource::Assistant;
    n6.content = QStringLiteral("项目结构已创建完成。接下来我们来实现主窗口类。");
    n6.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n6);

    // 7. 代码块
    TimelineNode n7;
    n7.id = generateId();
    n7.type = NodeType::CodeBlock;
    n7.language = QStringLiteral("cpp");
    n7.code = QStringLiteral(
        "#ifndef MAINWINDOW_H\n"
        "#define MAINWINDOW_H\n\n"
        "#include <QMainWindow>\n"
        "#include <QListView>\n\n"
        "class MainWindow : public QMainWindow {\n"
        "    Q_OBJECT\n"
        "public:\n"
        "    explicit MainWindow(QWidget* parent = nullptr);\n"
        "private:\n"
        "    QListView* m_taskList;\n"
        "};\n\n"
        "#endif");
    n7.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n7);

    // ==================== 对话 2: 用户反馈 ====================

    // 8. 用户消息
    TimelineNode n8;
    n8.id = generateId();
    n8.type = NodeType::UserMessage;
    n8.source = MessageSource::User;
    n8.content = QStringLiteral("这个头文件看起来不错，能继续实现 cpp 文件吗？");
    n8.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n8);

    // 9. 思考过程
    TimelineNode n9;
    n9.id = generateId();
    n9.type = NodeType::ThinkingStep;
    n9.thinkingSteps << QStringLiteral("检查头文件结构")
                     << QStringLiteral("实现构造函数")
                     << QStringLiteral("设置 UI 布局");
    n9.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n9);

    // 10. AI 消息
    TimelineNode n10;
    n10.id = generateId();
    n10.type = NodeType::AIMessage;
    n10.source = MessageSource::Assistant;
    n10.content = QStringLiteral("当然！这是对应的实现文件：");
    n10.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n10);

    // 11. 代码块
    TimelineNode n11;
    n11.id = generateId();
    n11.type = NodeType::CodeBlock;
    n11.language = QStringLiteral("cpp");
    n11.code = QStringLiteral(
        "#include \"mainwindow.h\"\n"
        "#include <QVBoxLayout>\n\n"
        "MainWindow::MainWindow(QWidget* parent)\n"
        "    : QMainWindow(parent)\n"
        "    , m_taskList(new QListView(this))\n"
        "{\n"
        "    setWindowTitle(tr(\"Task Manager\"));\n"
        "    setCentralWidget(m_taskList);\n"
        "    resize(800, 600);\n"
        "}");
    n11.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n11);

    // 12. 工具调用
    TimelineNode n12;
    n12.id = generateId();
    n12.type = NodeType::ToolCall;
    n12.toolName = QStringLiteral("write_file");
    n12.toolArgs = {{QStringLiteral("path"), QStringLiteral("src/mainwindow.cpp")}};
    n12.toolStatus = ToolStatus::Success;
    n12.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n12);

    // ==================== 对话 3: 功能扩展 ====================

    // 13. 用户消息
    TimelineNode n13;
    n13.id = generateId();
    n13.type = NodeType::UserMessage;
    n13.source = MessageSource::User;
    n13.content = QStringLiteral("我想添加一个工具栏，包含添加任务和删除任务的按钮");
    n13.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n13);

    // 14. 思考过程
    TimelineNode n14;
    n14.id = generateId();
    n14.type = NodeType::ThinkingStep;
    n14.thinkingSteps << QStringLiteral("设计工具栏布局")
                     << QStringLiteral("创建 QAction 对象")
                     << QStringLiteral("连接信号槽");
    n14.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n14);

    // 15. 任务列表
    TimelineNode n15;
    n15.id = generateId();
    n15.type = NodeType::TaskList;
    n15.tasks = {
        {generateId(), QStringLiteral("创建 QToolBar"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("添加 QAction - 新建任务"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("添加 QAction - 删除任务"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("实现槽函数"), TaskStatus::InProgress, 0},
        {generateId(), QStringLiteral("添加快捷键支持"), TaskStatus::Pending, 0}
    };
    n15.completedCount = 3;
    n15.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n15);

    // 16. AI 消息
    TimelineNode n16;
    n16.id = generateId();
    n16.type = NodeType::AIMessage;
    n16.source = MessageSource::Assistant;
    n16.content = QStringLiteral("好的，我来帮你添加工具栏。需要修改头文件和实现文件。");
    n16.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n16);

    // 17. 代码块
    TimelineNode n17;
    n17.id = generateId();
    n17.type = NodeType::CodeBlock;
    n17.language = QStringLiteral("cpp");
    n17.code = QStringLiteral(
        "void MainWindow::setupToolBar()\n"
        "{\n"
        "    QToolBar* toolbar = addToolBar(tr(\"Main\"));\n"
        "    \n"
        "    QAction* addAct = new QAction(tr(\"Add\"), this);\n"
        "    addAct->setIcon(QIcon::fromTheme(\"list-add\"));\n"
        "    connect(addAct, &QAction::triggered, this, &MainWindow::addTask);\n"
        "    \n"
        "    QAction* delAct = new QAction(tr(\"Delete\"), this);\n"
        "    delAct->setIcon(QIcon::fromTheme(\"list-remove\"));\n"
        "    connect(delAct, &QAction::triggered, this, &MainWindow::deleteTask);\n"
        "    \n"
        "    toolbar->addAction(addAct);\n"
        "    toolbar->addAction(delAct);\n"
        "}");
    n17.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n17);

    // ==================== 对话 4: 调试问题 ====================

    // 18. 用户消息
    TimelineNode n18;
    n18.id = generateId();
    n18.type = NodeType::UserMessage;
    n18.source = MessageSource::User;
    n18.content = QStringLiteral("编译时遇到错误：'QAction' was not declared in this scope");
    n18.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n18);

    // 19. 思考过程
    TimelineNode n19;
    n19.id = generateId();
    n19.type = NodeType::ThinkingStep;
    n19.thinkingSteps << QStringLiteral("分析编译错误")
                     << QStringLiteral("检查头文件包含")
                     << QStringLiteral("定位缺失的 include");
    n19.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n19);

    // 20. AI 消息
    TimelineNode n20;
    n20.id = generateId();
    n20.type = NodeType::AIMessage;
    n20.source = MessageSource::Assistant;
    n20.content = QStringLiteral("这是因为缺少 QAction 的头文件。需要在 cpp 文件顶部添加：");
    n20.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n20);

    // 21. 代码块
    TimelineNode n21;
    n21.id = generateId();
    n21.type = NodeType::CodeBlock;
    n21.language = QStringLiteral("cpp");
    n21.code = QStringLiteral(
        "#include <QAction>\n"
        "#include <QToolBar>\n"
        "#include <QIcon>");
    n21.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n21);

    // 22. 工具调用 - 读取文件
    TimelineNode n22;
    n22.id = generateId();
    n22.type = NodeType::ToolCall;
    n22.toolName = QStringLiteral("read_file");
    n22.toolArgs = {{QStringLiteral("path"), QStringLiteral("src/mainwindow.cpp")}};
    n22.toolStatus = ToolStatus::Success;
    n22.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n22);

    // 23. 工具调用 - 编辑文件
    TimelineNode n23;
    n23.id = generateId();
    n23.type = NodeType::ToolCall;
    n23.toolName = QStringLiteral("edit_file");
    n23.toolArgs = {{QStringLiteral("path"), QStringLiteral("src/mainwindow.cpp")}};
    n23.toolStatus = ToolStatus::Success;
    n23.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n23);

    // 24. AI 消息
    TimelineNode n24;
    n24.id = generateId();
    n24.type = NodeType::AIMessage;
    n24.source = MessageSource::Assistant;
    n24.content = QStringLiteral("已修复！重新编译应该就可以了。");
    n24.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n24);

    // ==================== 对话 5: 继续开发 ====================

    // 25. 用户消息
    TimelineNode n25;
    n25.id = generateId();
    n25.type = NodeType::UserMessage;
    n25.source = MessageSource::User;
    n25.content = QStringLiteral("编译成功了！现在想添加任务优先级功能，支持高、中、低三个级别");
    n25.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n25);

    // 26. 思考过程
    TimelineNode n26;
    n26.id = generateId();
    n26.type = NodeType::ThinkingStep;
    n26.thinkingSteps << QStringLiteral("设计优先级枚举")
                     << QStringLiteral("修改数据模型")
                     << QStringLiteral("更新 UI 显示")
                     << QStringLiteral("添加筛选功能");
    n26.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n26);

    // 27. 任务列表
    TimelineNode n27;
    n27.id = generateId();
    n27.type = NodeType::TaskList;
    n27.tasks = {
        {generateId(), QStringLiteral("定义 Priority 枚举"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("更新 Task 数据结构"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("实现优先级下拉框委托"), TaskStatus::InProgress, 0},
        {generateId(), QStringLiteral("添加优先级图标"), TaskStatus::Pending, 0},
        {generateId(), QStringLiteral("实现优先级排序"), TaskStatus::Pending, 0},
        {generateId(), QStringLiteral("添加筛选下拉框"), TaskStatus::Pending, 0}
    };
    n27.completedCount = 2;
    n27.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n27);

    // 28. AI 消息
    TimelineNode n28;
    n28.id = generateId();
    n28.type = NodeType::AIMessage;
    n28.source = MessageSource::Assistant;
    n28.content = QStringLiteral("好的！优先级功能是个很好的想法。我来帮你设计和实现。首先定义优先级枚举：");
    n28.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n28);

    // 29. 代码块
    TimelineNode n29;
    n29.id = generateId();
    n29.type = NodeType::CodeBlock;
    n29.language = QStringLiteral("cpp");
    n29.code = QStringLiteral(
        "enum class Priority {\n"
        "    High   = 0,\n"
        "    Medium = 1,\n"
        "    Low    = 2\n"
        "};\n\n"
        "struct Task {\n"
        "    QString title;\n"
        "    QString description;\n"
        "    Priority priority;\n"
        "    bool completed;\n"
        "};");
    n29.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n29);

    // 30. 工具调用
    TimelineNode n30;
    n30.id = generateId();
    n30.type = NodeType::ToolCall;
    n30.toolName = QStringLiteral("write_file");
    n30.toolArgs = {{QStringLiteral("path"), QStringLiteral("src/task.h")}};
    n30.toolStatus = ToolStatus::Success;
    n30.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n30);

    // 31. AI 消息
    TimelineNode n31;
    n31.id = generateId();
    n31.type = NodeType::AIMessage;
    n31.source = MessageSource::Assistant;
    n31.content = QStringLiteral("数据结构定义完成。接下来创建自定义委托来显示优先级下拉框：");
    n31.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n31);

    // 32. 代码块
    TimelineNode n32;
    n32.id = generateId();
    n32.type = NodeType::CodeBlock;
    n32.language = QStringLiteral("cpp");
    n32.code = QStringLiteral(
        "class PriorityDelegate : public QStyledItemDelegate {\n"
        "    Q_OBJECT\n"
        "public:\n"
        "    QWidget* createEditor(QWidget* parent,\n"
        "                          const QStyleOptionViewItem&,\n"
        "                          const QModelIndex&) const override {\n"
        "        QComboBox* combo = new QComboBox(parent);\n"
        "        combo->addItems({tr(\"High\"), tr(\"Medium\"), tr(\"Low\")});\n"
        "        return combo;\n"
        "    }\n"
        "};");
    n32.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n32);

    // ==================== 对话 6: 测试 ====================

    // 33. 用户消息
    TimelineNode n33;
    n33.id = generateId();
    n33.type = NodeType::UserMessage;
    n33.source = MessageSource::User;
    n33.content = QStringLiteral("功能都实现了，现在想写一些单元测试");
    n33.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n33);

    // 34. 思考过程
    TimelineNode n34;
    n34.id = generateId();
    n34.type = NodeType::ThinkingStep;
    n34.thinkingSteps << QStringLiteral("选择测试框架 (Qt Test)")
                     << QStringLiteral("规划测试用例")
                     << QStringLiteral("编写测试代码");
    n34.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n34);

    // 35. 任务列表
    TimelineNode n35;
    n35.id = generateId();
    n35.type = NodeType::TaskList;
    n35.tasks = {
        {generateId(), QStringLiteral("创建测试项目结构"), TaskStatus::Completed, 0},
        {generateId(), QStringLiteral("编写 TaskModel 测试"), TaskStatus::InProgress, 0},
        {generateId(), QStringLiteral("编写 Priority 测试"), TaskStatus::Pending, 0},
        {generateId(), QStringLiteral("编写序列化测试"), TaskStatus::Pending, 0}
    };
    n35.completedCount = 1;
    n35.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n35);

    // 36. AI 消息
    TimelineNode n36;
    n36.id = generateId();
    n36.type = NodeType::AIMessage;
    n36.source = MessageSource::Assistant;
    n36.content = QStringLiteral("好的！我来帮你创建单元测试。使用 Qt Test 框架：");
    n36.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n36);

    // 37. 代码块
    TimelineNode n37;
    n37.id = generateId();
    n37.type = NodeType::CodeBlock;
    n37.language = QStringLiteral("cpp");
    n37.code = QStringLiteral(
        "#include <QtTest/QtTest>\n"
        "#include \"taskmodel.h\"\n\n"
        "class TaskModelTest : public QObject {\n"
        "    Q_OBJECT\n"
        "private slots:\n"
        "    void testAddTask();\n"
        "    void testRemoveTask();\n"
        "    void testToggleComplete();\n"
        "};\n\n"
        "void TaskModelTest::testAddTask() {\n"
        "    TaskModel model;\n"
        "    QCOMPARE(model.rowCount(), 0);\n"
        "    model.addTask(Task{\"Test\", \"\", Priority::Medium});\n"
        "    QCOMPARE(model.rowCount(), 1);\n"
        "}");
    n37.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n37);

    // 38. 工具调用
    TimelineNode n38;
    n38.id = generateId();
    n38.type = NodeType::ToolCall;
    n38.toolName = QStringLiteral("write_file");
    n38.toolArgs = {{QStringLiteral("path"), QStringLiteral("tests/tst_taskmodel.cpp")}};
    n38.toolStatus = ToolStatus::Success;
    n38.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n38);

    // ==================== 对话 7: 运行测试 ====================

    // 39. 用户消息
    TimelineNode n39;
    n39.id = generateId();
    n39.type = NodeType::UserMessage;
    n39.source = MessageSource::User;
    n39.content = QStringLiteral("帮我运行一下测试");
    n39.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n39);

    // 40. 工具调用 - 运行命令
    TimelineNode n40;
    n40.id = generateId();
    n40.type = NodeType::ToolCall;
    n40.toolName = QStringLiteral("bash");
    n40.toolArgs = {{QStringLiteral("command"), QStringLiteral("cd build && ctest --output-on-failure")}};
    n40.toolStatus = ToolStatus::Running;
    n40.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n40);

    // 41. 工具调用 - 完成
    TimelineNode n41;
    n41.id = generateId();
    n41.type = NodeType::ToolCall;
    n41.toolName = QStringLiteral("bash");
    n41.toolArgs = {{QStringLiteral("command"), QStringLiteral("ctest")}};
    n41.toolStatus = ToolStatus::Success;
    n41.toolResult = {{QStringLiteral("passed"), 4}, {QStringLiteral("failed"), 0}};
    n41.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n41);

    // 42. AI 消息
    TimelineNode n42;
    n42.id = generateId();
    n42.type = NodeType::AIMessage;
    n42.source = MessageSource::Assistant;
    n42.content = QStringLiteral("测试全部通过！4 个测试用例都成功了。");
    n42.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n42);

    // ==================== 对话 8: 最终总结 ====================

    // 43. 用户消息
    TimelineNode n43;
    n43.id = generateId();
    n43.type = NodeType::UserMessage;
    n43.source = MessageSource::User;
    n43.content = QStringLiteral("项目基本完成了，能帮我生成一个 README 文件吗？");
    n43.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n43);

    // 44. 思考过程
    TimelineNode n44;
    n44.id = generateId();
    n44.type = NodeType::ThinkingStep;
    n44.thinkingSteps << QStringLiteral("收集项目信息")
                     << QStringLiteral("编写项目简介")
                     << QStringLiteral("添加使用说明")
                     << QStringLiteral("包含构建指南");
    n44.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n44);

    // 45. AI 消息
    TimelineNode n45;
    n45.id = generateId();
    n45.type = NodeType::AIMessage;
    n45.source = MessageSource::Assistant;
    n45.content = QStringLiteral("当然！这是一个完整的 README 文件：");
    n45.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n45);

    // 46. 代码块
    TimelineNode n46;
    n46.id = generateId();
    n46.type = NodeType::CodeBlock;
    n46.language = QStringLiteral("markdown");
    n46.code = QStringLiteral(
        "# Task Manager\n\n"
        "一个基于 Qt 的任务管理应用程序。\n\n"
        "## 功能\n\n"
        "- 添加/删除/编辑任务\n"
        "- 任务优先级（高/中/低）\n"
        "- 任务完成状态切换\n"
        "- 优先级筛选和排序\n\n"
        "## 构建\n\n"
        "```bash\n"
        "mkdir build && cd build\n"
        "cmake ..\n"
        "make\n"
        "```\n\n"
        "## 运行测试\n\n"
        "```bash\n"
        "cd build && ctest\n"
        "```");
    n46.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n46);

    // 47. 工具调用
    TimelineNode n47;
    n47.id = generateId();
    n47.type = NodeType::ToolCall;
    n47.toolName = QStringLiteral("write_file");
    n47.toolArgs = {{QStringLiteral("path"), QStringLiteral("README.md")}};
    n47.toolStatus = ToolStatus::Success;
    n47.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n47);

    // 48. AI 消息
    TimelineNode n48;
    n48.id = generateId();
    n48.type = NodeType::AIMessage;
    n48.source = MessageSource::Assistant;
    n48.content = QStringLiteral("README 文件已创建！项目现在已经完整了。有什么其他需要帮助的吗？");
    n48.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n48);

    // 49. 用户消息
    TimelineNode n49;
    n49.id = generateId();
    n49.type = NodeType::UserMessage;
    n49.source = MessageSource::User;
    n49.content = QStringLiteral("太棒了！项目结构清晰，功能完整。谢谢你的帮助！");
    n49.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n49);

    // 50. AI 消息
    TimelineNode n50;
    n50.id = generateId();
    n50.type = NodeType::AIMessage;
    n50.source = MessageSource::Assistant;
    n50.content = QStringLiteral("不客气！很高兴能帮助到你。如果后续有其他问题，随时可以问我。祝你的项目开发顺利！");
    n50.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_nodes.push_back(n50);

    endResetModel();
}

void TimelineModel::loadBulkTestData(int count)
{
    beginResetModel();
    m_nodes.clear();
    m_nodes.reserve(count * 3);  // 预留更多空间，因为一条对话可能包含多个节点

    QRandomGenerator* rng = QRandomGenerator::global();

    // 用户问题模板 - 更真实的问题
    struct UserQuestion {
        QString question;
        QString code;
        QString language;
    };

    QVector<UserQuestion> userQuestions = {
        {tr("你好，请帮我写一个排序算法"), "", ""},
        {tr("这段代码为什么报错？\n```cpp\nint* p = new int[10];\ndelete p;  // 应该是 delete[] p;\n```"), "int* p = new int[10];\ndelete p;", "cpp"},
        {tr("如何优化这个数据库查询？"), "", ""},
        {tr("帮我实现一个线程安全的单例模式"), "", ""},
        {tr("这个正则表达式怎么写？匹配邮箱地址"), "", ""},
        {tr("解释一下Qt的信号槽机制"), "", ""},
        {tr("如何实现一个HTTP服务器？"), "", ""},
        {tr("帮我写一个JSON解析器"), "", ""},
        {tr("这个内存泄漏怎么定位？"), "", ""},
        {tr("实现一个LRU缓存"), "", ""},
        {tr("如何处理大文件上传？"), "", ""},
        {tr("帮我设计一个状态机"), "", ""},
        {tr("这个递归会有栈溢出问题吗？"), "", ""},
        {tr("实现一个观察者模式"), "", ""},
        {tr("如何做单元测试mock？"), "", ""},
        {tr("这个算法的时间复杂度是多少？\n```python\ndef fib(n):\n    if n <= 1: return n\n    return fib(n-1) + fib(n-2)\n```"), "def fib(n):\n    if n <= 1: return n\n    return fib(n-1) + fib(n-2)", "python"},
        {tr("帮我写一个二叉树遍历"), "", ""},
        {tr("如何防止SQL注入？"), "", ""},
        {tr("实现一个简单的RPC框架"), "", ""},
        {tr("这个死锁怎么解决？"), "", ""}
    };

    // AI 回复模板 - 包含思考过程和代码
    struct AIResponse {
        QString thinking;
        QString content;
        QString code;
        QString language;
        QString toolName;
    };

    QVector<AIResponse> aiResponses = {
        {
            tr("分析需求 → 选择算法 → 考虑边界情况"),
            tr("好的，我来帮你实现一个快速排序算法。快速排序是一个高效的排序算法，平均时间复杂度为 O(n log n)。"),
            "void quickSort(vector<int>& arr, int left, int right) {\n    if (left >= right) return;\n    int pivot = arr[(left + right) / 2];\n    int i = left, j = right;\n    while (i <= j) {\n        while (arr[i] < pivot) i++;\n        while (arr[j] > pivot) j--;\n        if (i <= j) {\n            swap(arr[i], arr[j]);\n            i++; j--;\n        }\n    }\n    quickSort(arr, left, j);\n    quickSort(arr, i, right);\n}",
            "cpp", "write_file"
        },
        {
            tr("识别问题 → 分析原因 → 提供解决方案"),
            tr("你发现了一个常见的内存管理错误。当你使用 `new[]` 分配数组时，必须使用 `delete[]` 来释放，否则会导致内存泄漏。"),
            "// 正确的做法\nint* p = new int[10];\n// ... 使用数组 ...\ndelete[] p;  // 注意使用 delete[]\n\n// 或者更好的方式：使用智能指针\nauto p = std::make_unique<int[]>(10);",
            "cpp", "explain_code"
        },
        {
            tr("分析查询计划 → 识别瓶颈 → 优化建议"),
            tr("数据库查询优化需要从以下几个方面入手：\n\n1. **索引优化** - 确保查询字段有合适的索引\n2. **避免 SELECT *** - 只查询需要的字段\n3. **使用 EXPLAIN 分析** - 了解查询执行计划"),
            "-- 添加索引\nCREATE INDEX idx_user_email ON users(email);\n\n-- 优化查询\nSELECT id, name FROM users WHERE email = ?;",
            "sql", "execute_query"
        },
        {
            tr("分析线程安全需求 → 选择实现方式 → 考虑C++11特性"),
            tr("线程安全的单例模式推荐使用 Meyers' Singleton，利用C++11的静态局部变量线程安全特性："),
            "class Singleton {\npublic:\n    static Singleton& instance() {\n        static Singleton inst;  // C++11保证线程安全\n        return inst;\n    }\n    \n    // 禁止拷贝\n    Singleton(const Singleton&) = delete;\n    Singleton& operator=(const Singleton&) = delete;\n    \nprivate:\n    Singleton() = default;\n};",
            "cpp", "write_file"
        },
        {
            tr("理解邮箱格式 → 设计正则模式 → 测试验证"),
            tr("邮箱地址的正则表达式需要匹配以下格式：\n- 用户名：字母、数字、点、下划线、减号\n- @ 符号\n- 域名：字母、数字、点、减号"),
            "// 邮箱正则表达式\nconst QRegularExpression emailRegex(\n    R\"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$)\"\n);\n\n// 使用示例\nbool isValid = emailRegex.match(email).hasMatch();",
            "cpp", "write_file"
        },
        {
            tr("理解Qt元对象系统 → 分析连接机制 → 总结关键点"),
            tr("Qt的信号槽机制是其核心特性之一，它实现了对象间的松耦合通信：\n\n**工作原理：**\n1. 信号(Signal) - 事件发生时发射\n2. 槽(Slot) - 响应信号的函数\n3. 连接(Connection) - 建立信号和槽的关联\n\n**连接方式：**\n- 直接连接(Direct) - 同线程立即调用\n- 队列连接(Queued) - 跨线程异步调用"),
            "// 连接示例\nconnect(sender, &Sender::signalName,\n        receiver, &Receiver::slotName);\n\n// Lambda连接\nconnect(button, &QPushButton::clicked, [=]() {\n    qDebug() << \"Button clicked!\";\n});",
            "cpp", ""
        },
        {
            tr("设计服务器架构 → 选择协议 → 实现核心功能"),
            tr("实现一个简单的HTTP服务器可以使用Qt的QTcpServer：\n\n核心步骤：\n1. 创建TCP服务器监听端口\n2. 接受客户端连接\n3. 解析HTTP请求\n4. 发送HTTP响应"),
            "class HttpServer : public QTcpServer {\n    void incomingConnection(qintptr socketDesc) override {\n        QTcpSocket* socket = new QTcpSocket(this);\n        socket->setSocketDescriptor(socketDesc);\n        connect(socket, &QTcpSocket::readyRead, [=]() {\n            QByteArray request = socket->readAll();\n            // 解析请求...\n            QByteArray response = \"HTTP/1.1 200 OK\\r\\n\\r\\nHello!\";\n            socket->write(response);\n            socket->disconnectFromHost();\n        });\n    }\n};",
            "cpp", "write_file"
        },
        {
            tr("分析JSON结构 → 设计解析器 → 处理错误情况"),
            tr("使用Qt内置的QJsonDocument来解析JSON非常简单："),
            "// 解析JSON\nQByteArray jsonData = R\"({\"name\": \"Test\", \"value\": 42})\";\nQJsonDocument doc = QJsonDocument::fromJson(jsonData);\n\nif (doc.isObject()) {\n    QJsonObject obj = doc.object();\n    QString name = obj[\"name\"].toString();\n    int value = obj[\"value\"].toInt();\n}",
            "cpp", ""
        },
        {
            tr("定位内存泄漏原因 → 选择检测工具 → 提供解决方案"),
            tr("内存泄漏定位方法：\n\n1. **Valgrind** (Linux) - 最强大的内存检测工具\n2. **Visual Studio Debugger** (Windows) - 内置内存快照功能\n3. **AddressSanitizer** (GCC/Clang) - 编译时检测\n4. **Qt对象树** - 检查QObject父子关系"),
            "// 编译时启用AddressSanitizer\n// g++ -fsanitize=address -g program.cpp\n\n// 运行时会自动检测内存泄漏",
            "bash", "run_command"
        },
        {
            tr("理解LRU原理 → 设计数据结构 → 实现get/put操作"),
            tr("LRU(Least Recently Used)缓存需要：\n1. 哈希表 - O(1)查找\n2. 双向链表 - O(1)移动节点\n\n最近使用的移到头部，满时删除尾部"),
            "class LRUCache {\n    list<pair<int,int>> cache;  // (key, value)\n    unordered_map<int, list<pair<int,int>>::iterator> map;\n    int capacity;\npublic:\n    int get(int key) {\n        if (!map.count(key)) return -1;\n        cache.splice(cache.begin(), cache, map[key]);\n        return map[key]->second;\n    }\n    void put(int key, int value) {\n        if (map.count(key)) {\n            cache.erase(map[key]);\n        } else if (cache.size() >= capacity) {\n            map.erase(cache.back().first);\n            cache.pop_back();\n        }\n        cache.push_front({key, value});\n        map[key] = cache.begin();\n    }\n};",
            "cpp", "write_file"
        }
    };

    // 工具调用模板
    QVector<QString> toolNames = {
        QStringLiteral("read_file"),
        QStringLiteral("write_file"),
        QStringLiteral("execute_command"),
        QStringLiteral("search_web"),
        QStringLiteral("list_directory"),
        QStringLiteral("analyze_code")
    };

    qint64 baseTime = QDateTime::currentMSecsSinceEpoch() - count * 5000;
    int conversationIndex = 0;

    for (int i = 0; i < count; ++i) {
        int questionIdx = conversationIndex % userQuestions.size();
        const auto& question = userQuestions[questionIdx];
        int responseIdx = conversationIndex % aiResponses.size();
        const auto& response = aiResponses[responseIdx];

        // 1. 用户消息
        TimelineNode userNode;
        userNode.id = generateId();
        userNode.type = NodeType::UserMessage;
        userNode.source = MessageSource::User;
        userNode.content = question.question;
        userNode.timestamp = baseTime + i * 3000;
        m_nodes.push_back(userNode);

        // 2. 思考过程（30%概率添加）
        if (rng->bounded(100) < 30) {
            TimelineNode thinkNode;
            thinkNode.id = generateId();
            thinkNode.type = NodeType::ThinkingStep;
            thinkNode.thinkingSteps = response.thinking.split(QStringLiteral(" → "));
            thinkNode.timestamp = baseTime + i * 3000 + 500;
            m_nodes.push_back(thinkNode);
        }

        // 3. 工具调用（40%概率添加）
        if (rng->bounded(100) < 40 && !response.toolName.isEmpty()) {
            TimelineNode toolNode;
            toolNode.id = generateId();
            toolNode.type = NodeType::ToolCall;
            toolNode.toolName = response.toolName;
            toolNode.toolArgs = {{QStringLiteral("file"), QStringLiteral("src/implementation.cpp")}};
            toolNode.toolStatus = static_cast<ToolStatus>(rng->bounded(3));  // Pending/Running/Success
            toolNode.timestamp = baseTime + i * 3000 + 1000;
            m_nodes.push_back(toolNode);
        }

        // 4. AI消息（带代码块）
        TimelineNode aiNode;
        aiNode.id = generateId();
        aiNode.type = NodeType::AIMessage;
        aiNode.source = MessageSource::Assistant;
        aiNode.content = response.content;
        aiNode.timestamp = baseTime + i * 3000 + 1500;
        m_nodes.push_back(aiNode);

        // 5. 代码块（60%概率添加）
        if (rng->bounded(100) < 60 && !response.code.isEmpty()) {
            TimelineNode codeNode;
            codeNode.id = generateId();
            codeNode.type = NodeType::CodeBlock;
            codeNode.language = response.language;
            codeNode.code = response.code;
            codeNode.timestamp = baseTime + i * 3000 + 2000;
            m_nodes.push_back(codeNode);
        }

        conversationIndex++;
    }

    endResetModel();
}

QModelIndex TimelineModel::indexForNodeId(const QString& nodeId) const
{
    for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
        if (m_nodes[i].id == nodeId) {
            return index(i);
        }
    }
    return QModelIndex();
}

// ============================================================================
// 私有方法
// ============================================================================

QString TimelineModel::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void TimelineModel::updateCompletedCount(TimelineNode& node)
{
    node.completedCount = 0;
    for (const auto& task : node.tasks) {
        if (task.status == TaskStatus::Completed) {
            ++node.completedCount;
        }
    }
}

} // namespace Timeline
