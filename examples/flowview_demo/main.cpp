/**
 * @file main.cpp
 * @brief FlowView 高性能虚拟化列表演示
 *
 * 本示例展示如何使用 FlowView 组件显示大量数据（100k+ 项），
 * 并保持流畅的滚动性能。
 *
 * ## 功能演示
 * - 聊天消息列表演示
 * - 变高度项支持
 * - 键盘导航
 * - 流畅滚动
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QElapsedTimer>

#include <vlayout/framework.h>
#include <views/flowview/flowview.h>

#include "chat_delegate.h"
#include "chat_model.h"

/**
 * @class MainWindow
 * @brief 主窗口，展示 FlowView 的使用
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle(tr("FlowView 演示 - 高性能虚拟化列表"));
        resize(800, 600);

        setupUI();
        setupConnections();

        // 加载初始数据 - 10万条数据测试
        populateModel(100000);
    }

private slots:
    void onSendClicked()
    {
        QString text = m_inputEdit->text().trimmed();
        if (text.isEmpty()) return;

        // 添加用户消息
        m_model->addMessage(text, MessageType::User);
        m_inputEdit->clear();

        // 模拟 AI 回复
        QTimer::singleShot(500, this, [this, text]() {
            m_model->addMessage(tr("收到: %1").arg(text), MessageType::Assistant);
            m_flowView->scrollToBottom();
        });

        // 滚动到底部
        m_flowView->scrollToBottom();
    }

    void onPopulateClicked()
    {
        int count = m_countSpin->value();
        populateModel(count);
    }

    void onClearClicked()
    {
        m_model->clear();
    }

private:
    void setupUI()
    {
        // 中心部件
        auto* central = new QWidget(this);
        setCentralWidget(central);

        auto* layout = new QVBoxLayout(central);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // 工具栏
        auto* toolbar = new QToolBar(tr("工具栏"), this);
        addToolBar(toolbar);

        auto* countLabel = new QLabel(tr("数据量: "), this);
        toolbar->addWidget(countLabel);

        m_countSpin = new QSpinBox(this);
        m_countSpin->setRange(10, 1000000);
        m_countSpin->setValue(100000);
        m_countSpin->setSingleStep(10000);
        toolbar->addWidget(m_countSpin);

        auto* populateBtn = new QPushButton(tr("生成数据"), this);
        populateBtn->setStyleSheet("QPushButton { font-weight: bold; padding: 6px 12px; }");
        toolbar->addWidget(populateBtn);
        connect(populateBtn, &QPushButton::clicked, this, &MainWindow::onPopulateClicked);

        // 快速 10万测试按钮
        auto* quickTestBtn = new QPushButton(tr("10万数据测试"), this);
        toolbar->addWidget(quickTestBtn);
        connect(quickTestBtn, &QPushButton::clicked, this, [this]() {
            m_countSpin->setValue(100000);
            populateModel(100000);
        });

        toolbar->addSeparator();

        auto* clearBtn = new QPushButton(tr("清空"), this);
        toolbar->addWidget(clearBtn);
        connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearClicked);

        // 状态标签
        m_statusLabel = new QLabel(tr("就绪"), this);
        toolbar->addWidget(m_statusLabel);

        // FlowView - 使用 VLayout 的 DelegateController
        m_flowView = new VLayout::FlowView(this);
        m_delegate = new ChatDelegate(m_flowView);
        m_flowView->setDelegate(m_delegate);

        layout->addWidget(m_flowView);

        // 输入区域
        auto* inputContainer = new QWidget(this);
        inputContainer->setStyleSheet("QWidget { background: #f5f5f5; padding: 8px; }");
        auto* inputLayout = new QHBoxLayout(inputContainer);
        inputLayout->setContentsMargins(8, 8, 8, 8);

        m_inputEdit = new QLineEdit(inputContainer);
        m_inputEdit->setPlaceholderText(tr("输入消息..."));
        m_inputEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ddd; border-radius: 4px; }");

        auto* sendBtn = new QPushButton(tr("发送"), inputContainer);
        sendBtn->setStyleSheet("QPushButton { padding: 8px 16px; background: #0078d4; color: white; border: none; border-radius: 4px; }"
                               "QPushButton:hover { background: #106ebe; }");

        inputLayout->addWidget(m_inputEdit);
        inputLayout->addWidget(sendBtn);

        layout->addWidget(inputContainer);

        // 连接发送按钮
        connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
        connect(m_inputEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);

        // 连接 FlowView 信号
        connect(m_flowView, &VLayout::FlowView::clicked, this, [this](int index) {
            m_statusLabel->setText(tr("点击: 索引 %1 / 总数 %2").arg(index).arg(m_flowView->count()));
        });

        connect(m_flowView, &VLayout::FlowView::countChanged, this, [this](int count) {
            m_statusLabel->setText(tr("✅ 已加载 %1 条数据 - 滚动流畅").arg(count));
        });
    }

    void setupConnections()
    {
        // 其他连接
    }

    void populateModel(int count)
    {
        QElapsedTimer timer;

        m_statusLabel->setText(tr("⏳ 正在生成 %1 条数据...").arg(count));
        m_statusLabel->repaint();
        QApplication::processEvents();

        // 生成数据
        timer.start();
        m_model->populate(count);
        qint64 genTime = timer.elapsed();

        // 设置模型到视图
        timer.restart();
        m_flowView->setModel(m_model);
        m_flowView->scrollToBottom();
        qint64 setupTime = timer.elapsed();

        m_statusLabel->setText(tr("✅ 已加载 %1 条数据 | 生成: %2ms | 显示: %3ms | 滚动流畅")
                        .arg(count)
                        .arg(genTime)
                        .arg(setupTime));
    }

private:
    VLayout::FlowView* m_flowView = nullptr;
    ChatDelegate* m_delegate = nullptr;
    ChatModel* m_model = new ChatModel(this);
    QLineEdit* m_inputEdit = nullptr;
    QLabel* m_statusLabel = nullptr;
    QSpinBox* m_countSpin = nullptr;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用样式
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
