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
#include <QTimer>
#include <QDebug>

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
        setWindowTitle(tr("Timeline Demo (Ollama)"));
        resize(900, 700);

        // 初始化适配器
        setupAdapter();

        // 加载示例数据
        QTimer::singleShot(500, this, [this]() {
            m_timeline->loadSampleData();
        });
    }

private:
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
