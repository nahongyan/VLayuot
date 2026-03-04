#include "timeline_widget.h"
#include "timeline_model.h"
#include "timeline_delegate.h"
#include "timeline_theme.h"

#include <QListView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTimer>

namespace Timeline {

// ============================================================================
// TimelineWidget
// ============================================================================

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupConnections();
    setupStyleSheet();
}

TimelineWidget::~TimelineWidget()
{
}

// ============================================================================
// 操作接口
// ============================================================================

void TimelineWidget::addUserMessage(const QString& content)
{
    m_model->addUserMessage(content);
    scrollToBottom();
}

void TimelineWidget::addAIMessage(const QString& content, bool streaming)
{
    m_model->addAIMessage(content, streaming);
    scrollToBottom();
}

void TimelineWidget::updateAIMessage(const QString& content)
{
    m_model->updateLastAIMessage(content);
    scrollToBottom();
}

void TimelineWidget::finalizeAIMessage(const QString& content)
{
    m_model->updateLastAIMessage(content, false);
    scrollToBottom();
}

void TimelineWidget::addCodeBlock(const QString& language, const QString& code)
{
    m_model->addCodeBlock(language, code);
    scrollToBottom();
}

QString TimelineWidget::addToolCall(const QString& toolName, const QVariantMap& args)
{
    QString nodeId = m_model->addToolCall(toolName, args);
    scrollToBottom();
    return nodeId;
}

void TimelineWidget::updateToolResult(const QString& nodeId, const QString& result)
{
    m_model->setToolResult(nodeId, QVariantMap{{QStringLiteral("content"), result}});
    m_model->updateToolStatus(nodeId, ToolStatus::Success);
}

void TimelineWidget::addThinking(const QStringList& steps)
{
    m_model->addThinking(steps);
    scrollToBottom();
}

QString TimelineWidget::startThinking()
{
    QString nodeId = m_model->addThinking(QStringList());
    m_model->setThinkingState(nodeId, true);
    scrollToBottom();
    return nodeId;
}

void TimelineWidget::updateThinking(const QString& nodeId, const QString& content)
{
    m_model->updateThinkingContent(nodeId, content);
    scrollToBottom();
}

void TimelineWidget::finalizeThinking(const QString& nodeId)
{
    m_model->setThinkingState(nodeId, false);
}

void TimelineWidget::addTaskList(const std::vector<TaskItem>& tasks)
{
    m_model->addTaskList(tasks);
    scrollToBottom();
}

void TimelineWidget::clear()
{
    m_model->clear();
}

void TimelineWidget::loadSampleData()
{
    m_model->loadSampleData();
    scrollToBottom();
}

void TimelineWidget::scrollToBottom()
{
    QTimer::singleShot(100, this, [this]() {
        m_listView->scrollToBottom();
    });
}

// ============================================================================
// UI 设置
// ============================================================================

void TimelineWidget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 时间轴列表视图 ==========

    m_listView = new QListView(this);
    m_listView->setObjectName(QStringLiteral("timelineList"));

    // 基本设置
    m_listView->setFrameShape(QFrame::NoFrame);
    m_listView->setSelectionMode(QAbstractItemView::NoSelection);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 启用鼠标追踪以支持悬停效果
    m_listView->setMouseTracking(true);
    m_listView->viewport()->setAttribute(Qt::WA_Hover);

    mainLayout->addWidget(m_listView, 1);

    // ========== 输入区域 ==========

    m_inputContainer = new QWidget(this);
    m_inputContainer->setObjectName(QStringLiteral("inputContainer"));
    m_inputContainer->setFixedHeight(60);

    QHBoxLayout* inputLayout = new QHBoxLayout(m_inputContainer);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(12);

    // 输入框
    m_inputEdit = new QLineEdit(m_inputContainer);
    m_inputEdit->setObjectName(QStringLiteral("inputEdit"));
    m_inputEdit->setPlaceholderText(tr("Type a message..."));
    m_inputEdit->setClearButtonEnabled(true);
    inputLayout->addWidget(m_inputEdit, 1);

    // 发送按钮
    QPushButton* sendBtn = new QPushButton(tr("Send"), m_inputContainer);
    sendBtn->setObjectName(QStringLiteral("sendBtn"));
    sendBtn->setFixedSize(80, 36);
    inputLayout->addWidget(sendBtn);

    mainLayout->addWidget(m_inputContainer);

    // ========== 模型和委托 ==========

    m_model = new TimelineModel(this);
    m_listView->setModel(m_model);

    m_delegate = new TimelineDelegate(this);
    m_listView->setItemDelegate(m_delegate);
}

void TimelineWidget::setupConnections()
{
    // 发送按钮
    QPushButton* sendBtn = findChild<QPushButton*>(QStringLiteral("sendBtn"));
    if (sendBtn) {
        connect(sendBtn, &QPushButton::clicked, this, &TimelineWidget::onSendClicked);
    }

    // 输入框回车
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TimelineWidget::onSendClicked);

    // 委托信号
    connect(m_delegate, &TimelineDelegate::copyCodeRequested,
            this, &TimelineWidget::onCopyCodeRequested);
    connect(m_delegate, &TimelineDelegate::expandedChanged,
            this, &TimelineWidget::onExpandedChanged);
}

void TimelineWidget::setupStyleSheet()
{
    // 扁平化设计样式表 - 无 hover 效果
    QString style = QStringLiteral(R"(
        /* 主容器 */
        TimelineWidget {
            background-color: %1;
        }

        /* 时间轴列表 */
        #timelineList {
            background-color: %1;
            outline: none;
            border: none;
        }

        #timelineList::item {
            border: none;
            outline: none;
            padding: 0;
            background-color: transparent;
        }

        #timelineList::item:hover {
            background-color: transparent;
        }

        #timelineList::item:selected {
            background-color: transparent;
        }

        /* 输入区域 */
        #inputContainer {
            background-color: %2;
            border-top: 1px solid %3;
        }

        /* 输入框 */
        #inputEdit {
            background-color: %4;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 8px 12px;
            color: %5;
            font-size: 10pt;
            selection-background-color: %6;
        }

        #inputEdit:focus {
            border-color: %7;
        }

        #inputEdit::placeholder {
            color: %8;
        }

        /* 发送按钮 */
        #sendBtn {
            background-color: %7;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 10pt;
            font-weight: bold;
        }

        #sendBtn:hover {
            background-color: %9;
        }

        #sendBtn:pressed {
            background-color: %10;
        }

        /* 滚动条 - 更宽更明显 */
        QScrollBar:vertical {
            background-color: rgba(60, 60, 60, 100);
            width: %13px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background-color: %11;
            min-height: 40px;
            border-radius: %14px;
            margin: 2px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %12;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )").arg(
        Theme::bgBase.name(),                      // %1 - 主背景
        Theme::bgInput.name(),                     // %2 - 输入区背景
        Theme::separator.name(),                   // %3 - 边框
        Theme::bgBase.name(),                      // %4 - 输入框背景
        Theme::textPrimary.name(),                 // %5 - 主文本色
        QColor(217, 119, 87, 50).name(),           // %6 - 选中背景 (半透明)
        Theme::accentAI.name(),                    // %7 - 强调色
        Theme::textSecond.name(),                  // %8 - 次要文本色
        QColor(217, 119, 87).lighter(110).name(),  // %9 - 按钮 hover
        QColor(217, 119, 87).darker(110).name(),   // %10 - 按钮 pressed
        QColor(100, 100, 100).name(),              // %11 - 滚动条滑块 (更亮)
        QColor(140, 140, 140).name(),              // %12 - 滚动条 hover (更亮)
        QString::number(Theme::scrollBarWidth),    // %13 - 滚动条宽度
        QString::number(Theme::borderRadius)       // %14 - 圆角半径
    );

    setStyleSheet(style);
}

// ============================================================================
// 私有槽函数
// ============================================================================

void TimelineWidget::onSendClicked()
{
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    // 添加用户消息
    addUserMessage(text);

    // 清空输入框
    m_inputEdit->clear();

    // 发送信号
    emit messageSent(text);
}

void TimelineWidget::onCopyCodeRequested(const QString& nodeId, const QString& code)
{
    Q_UNUSED(nodeId)
    Q_UNUSED(code)

    // 已经在 delegate 中复制到剪贴板
    emit codeCopied(nodeId, code);
}

void TimelineWidget::onExpandedChanged(const QString& nodeId, bool expanded)
{
    emit expandChanged(nodeId, expanded);
}

} // namespace Timeline
