#include "timeline_model.h"

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
