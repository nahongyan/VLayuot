#ifndef TIMELINE_WIDGET_H
#define TIMELINE_WIDGET_H

/**
 * @file timeline_widget.h
 * @brief 时间轴主窗口组件定义
 *
 * 定义了 AI 编程助手时间轴的主窗口组件。
 */

#include <QWidget>
#include <QVariantMap>
#include <vector>

namespace VLayout {
class FlowView;
}

class QLineEdit;
class QLabel;

namespace Timeline {

class TimelineModel;
class TimelineDelegate;
struct TaskItem;  // 前向声明 TaskItem

/**
 * @class TimelineWidget
 * @brief 时间轴主组件
 *
 * 显示 AI 编程助手的时间轴界面。
 *
 * ## 布局
 * ┌─────────────────────────────────────┐
 * │ [输入框]                    [发送] │
 * ├─────────────────────────────────────┤
 * │                                     │
 * │  [时间轴节点列表]                   │
 * │                                     │
 * │  ┌─────────────────────────────┐   │
 * │  │ User: 请帮我实现排序         │   │
 * │  └─────────────────────────────┘   │
 * │                                     │
 * │  ┌─────────────────────────────┐   │
 * │  │ AI: 好的，我来帮你...       │   │
 * │  └─────────────────────────────┘   │
 * │                                     │
 * │  ...更多节点...                     │
 * │                                     │
 * └─────────────────────────────────────┘
 *
 * ## 功能
 * - 显示时间轴节点列表
 * - 支持滚动浏览
 * - 底部输入框
 * - 发送按钮
 */
class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override;

    // ========== 数据访问 ==========

    /**
     * @brief 获取数据模型
     */
    TimelineModel* model() const { return m_model; }

    // ========== 操作接口 ==========

    /**
     * @brief 添加用户消息
     */
    void addUserMessage(const QString& content);

    /**
     * @brief 添加 AI 消息
     */
    void addAIMessage(const QString& content, bool streaming = false);

    /**
     * @brief 更新 AI 消息（流式更新）
     */
    void updateAIMessage(const QString& content);

    /**
     * @brief 完成 AI 消息
     */
    void finalizeAIMessage(const QString& content);

    /**
     * @brief 添加代码块
     */
    void addCodeBlock(const QString& language, const QString& code);

    /**
     * @brief 添加工具调用
     * @param toolName 工具名称
     * @param args 工具参数
     * @return 工具节点 ID
     */
    QString addToolCall(const QString& toolName, const QVariantMap& args);

    /**
     * @brief 更新工具执行结果
     * @param nodeId 工具节点 ID
     * @param result 执行结果
     */
    void updateToolResult(const QString& nodeId, const QString& result);

    /**
     * @brief 添加思考过程
     */
    void addThinking(const QStringList& steps);

    /**
     * @brief 开始思考过程（流式）
     * @return 思考节点 ID
     */
    QString startThinking();

    /**
     * @brief 更新思考内容（流式）
     * @param nodeId 思考节点 ID
     * @param content 思考内容
     */
    void updateThinking(const QString& nodeId, const QString& content);

    /**
     * @brief 完成思考过程
     * @param nodeId 思考节点 ID
     */
    void finalizeThinking(const QString& nodeId);

    /**
     * @brief 添加任务列表
     */
    void addTaskList(const std::vector<TaskItem>& tasks);

    /**
     * @brief 清空时间轴
     */
    void clear();

    /**
     * @brief 加载示例数据
     */
    void loadSampleData();

    /**
     * @brief 加载大量测试数据
     * @param count 数据量
     */
    void loadBulkTestData(int count = 100000);

    /**
     * @brief 滚动到底部
     */
    void scrollToBottom();

signals:
    /**
     * @brief 用户发送消息
     */
    void messageSent(const QString& content);

    /**
     * @brief 复制代码
     */
    void codeCopied(const QString& nodeId, const QString& code);

    /**
     * @brief 折叠状态改变
     */
    void expandChanged(const QString& nodeId, bool expanded);

private:
    void setupUI();
    void setupConnections();
    void setupStyleSheet();

private slots:
    void onSendClicked();
    void onCopyCodeRequested(const QString& nodeId, const QString& code);
    void onExpandedChanged(const QString& nodeId, bool expanded);

private:
    // ========== UI 组件 ==========
    VLayout::FlowView* m_listView = nullptr;  ///< 时间轴列表视图 (使用 FlowView 实现高性能虚拟化渲染)
    QLineEdit* m_inputEdit = nullptr;         ///< 输入框
    QWidget* m_inputContainer = nullptr;      ///< 输入区域容器

    // ========== 数据 ==========
    TimelineModel* m_model = nullptr;         ///< 数据模型
    TimelineDelegate* m_delegate = nullptr;   ///< 委托
};

} // namespace Timeline

#endif // TIMELINE_WIDGET_H
