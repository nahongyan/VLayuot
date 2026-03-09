/**
 * @file main.cpp
 * @brief Timeline 组件演示（带 AI 支持）
 *
 * 使用 Ollama API 进行本地大模型对话。
 */

#include "timeline_widget.h"
#include "adapters/adapter.h"
#include "adapters/ollama_adapter.h"

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
    {
        // 创建时间轴组件
        m_timeline = new Timeline::TimelineWidget(this);
        setCentralWidget(m_timeline);

        // 设置窗口属性
        setWindowTitle(tr("Timeline Demo (FlowView 高性能虚拟化)"));
        resize(900, 700);

        // 创建工具栏
        setupToolBar();

        // 初始化适配器
        setupAdapter();

        // 加载大数据测试
        QTimer::singleShot(100, this, [this]() {
            loadBulkTestData(100000);
        });
    }

private:
    void setupToolBar()
    {
        QToolBar* toolbar = addToolBar(tr("测试工具"));

        // 数据量选择
        toolbar->addWidget(new QLabel(tr("数据量: ")));

        m_countSpin = new QSpinBox(this);
        m_countSpin->setRange(1000, 1000000);
        m_countSpin->setValue(100000);
        m_countSpin->setSingleStep(10000);
        toolbar->addWidget(m_countSpin);

        // 加载测试数据按钮
        auto* loadBtn = new QPushButton(tr("加载测试数据"), this);
        connect(loadBtn, &QPushButton::clicked, this, [this]() {
            int count = m_countSpin->value();
            loadBulkTestData(count);
        });
        toolbar->addWidget(loadBtn);

        // 快捷按钮
        auto* quick100kBtn = new QPushButton(tr("🚀 10万数据测试"), this);
        quick100kBtn->setStyleSheet("QPushButton { background: #e91e63; color: white; font-weight: bold; padding: 6px 12px; border-radius: 4px; }"
                                    "QPushButton:hover { background: #c73657; }");
        connect(quick100kBtn, &QPushButton::clicked, this, [this]() {
            m_countSpin->setValue(100000);
            loadBulkTestData(100000);
        });
        toolbar->addWidget(quick100kBtn);

        toolbar->addSeparator();

        // 状态标签
        m_statusLabel = new QLabel(tr("就绪"), this);
        toolbar->addWidget(m_statusLabel);
    }

    void loadBulkTestData(int count)
    {
        QElapsedTimer timer;
        timer.start();

        m_statusLabel->setText(tr("⏳ 正在加载 %1 条数据...").arg(count));
        m_statusLabel->repaint();
        QApplication::processEvents();

        m_timeline->loadBulkTestData(count);
        m_timeline->scrollToBottom();

        qint64 loadTime = timer.elapsed();
        m_statusLabel->setText(tr("✅ 已加载 %1 条数据 | 耗时: %2ms | 滚动流畅")
                        .arg(count)
                        .arg(loadTime));
    }

    void setupAdapter()
    {
        // 创建 Ollama 适配器
        m_adapter = new AISDK::OllamaAdapter(this);

        // 配置
        AISDK::AdapterConfig config;
        config.model = QStringLiteral("qwen3.5:27b");  // 使用 qwen3.5:27b 模型
        config.apiUrl = QStringLiteral("http://localhost:11434");

        // 连接信号
        connect(m_adapter, &AISDK::OllamaAdapter::readyChanged, this, [this](bool ready) {
            qDebug() << "[MainWindow] Adapter ready:" << ready;
            if (ready) {
                m_timeline->addAIMessage(QStringLiteral("✅ AI 服务已就绪 (Ollama + qwen3.5:27b)"), false);
            } else {
                m_timeline->addAIMessage(QStringLiteral("⚠️ 请确保 Ollama 服务正在运行: ollama serve"), false);
            }
        });

        connect(m_adapter, &AISDK::OllamaAdapter::thinkingReceived, this, [this](const QString& thinking) {
            qDebug() << "[MainWindow] Thinking:" << thinking.left(30);
            if (m_thinkingId.isEmpty()) {
                m_thinkingId = m_timeline->startThinking();
            }
            m_timeline->updateThinking(m_thinkingId, thinking);
        });

        connect(m_adapter, &AISDK::OllamaAdapter::partialTextReceived, this, [this](const QString& text) {
            // 第一次收到内容时添加 AI 消息节点
            if (!m_aiMessageAdded) {
                m_timeline->addAIMessage(QString(), true);
                m_aiMessageAdded = true;
            }
            qDebug() << "[MainWindow] Partial:" << text.left(50);
            m_timeline->updateAIMessage(text);
        });

        connect(m_adapter, &AISDK::OllamaAdapter::fullResponseReceived, this, [this](const QString& text) {
            qDebug() << "[MainWindow] Full response:" << text.length() << "chars";
            m_timeline->finalizeAIMessage(text);
        });

        connect(m_adapter, &AISDK::OllamaAdapter::errorOccurred, this, [this](const QString& error) {
            m_timeline->finalizeAIMessage(QStringLiteral("❌ 错误: %1").arg(error));
        });

        // 连接用户消息
        connect(m_timeline, &Timeline::TimelineWidget::messageSent, this, [this](const QString& content) {
            qDebug() << "[MainWindow] User:" << content;

            m_thinkingId.clear();
            m_aiMessageAdded = false;

            if (m_adapter->isReady()) {
                m_adapter->chat(content);
            } else {
                m_timeline->addAIMessage(QStringLiteral("⚠️ AI 服务未就绪"), false);
            }
        });

        // 初始化适配器
        m_adapter->initialize(config);
    }

private:
    Timeline::TimelineWidget* m_timeline = nullptr;
    AISDK::OllamaAdapter* m_adapter = nullptr;
    QString m_thinkingId;
    bool m_aiMessageAdded = false;

    // 测试控件
    QSpinBox* m_countSpin = nullptr;
    QLabel* m_statusLabel = nullptr;
};

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Timeline Demo"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
