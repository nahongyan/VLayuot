/**
 * @file main.cpp
 * @brief AI 编程助手时间轴组件演示
 *
 * 展示时间轴组件的基本用法，包括：
 * - 用户消息和 AI 消息显示
 * - 代码块展示
 * - 工具调用卡片
 * - 思考过程折叠面板
 * - 任务列表
 */

#include "timeline_widget.h"

#include <QApplication>
#include <QMainWindow>
#include <QTimer>

/**
 * @brief 主窗口类
 *
 * 包装 TimelineWidget 为独立窗口进行演示。
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
        setWindowTitle(tr("AI Assistant Timeline Demo"));
        resize(800, 600);

        // 连接信号
        connect(m_timeline, &Timeline::TimelineWidget::messageSent,
                this, &MainWindow::onMessageSent);

        // 加载示例数据
        QTimer::singleShot(500, this, [this]() {
            m_timeline->loadSampleData();
        });
    }

private slots:
    void onMessageSent(const QString& content)
    {
        // 模拟 AI 回复
        QTimer::singleShot(1000, this, [this, content]() {
            // 简单的回复逻辑
            QString reply;

            if (content.contains(QStringLiteral("排序")) ||
                content.contains(QStringLiteral("sort"))) {
                reply = tr("好的，我可以帮你实现排序算法。请问你需要哪种排序？"
                          "快速排序、归并排序还是冒泡排序？");
            } else if (content.contains(QStringLiteral("代码")) ||
                       content.contains(QStringLiteral("code"))) {
                reply = tr("我理解你需要代码相关的帮助。请告诉我具体需求，"
                          "我会为你生成相应的代码。");
            } else if (content.contains(QStringLiteral("帮助")) ||
                       content.contains(QStringLiteral("help"))) {
                reply = tr("我是一个 AI 编程助手，可以帮你：\n"
                          "• 编写和修改代码\n"
                          "• 解释代码逻辑\n"
                          "• 调试和修复 Bug\n"
                          "• 代码重构和优化\n"
                          "请告诉我你需要什么帮助！");
            } else {
                reply = tr("收到你的消息：\"%1\"\n我正在思考如何帮助你...").arg(content);
            }

            m_timeline->addAIMessage(reply);
        });
    }

private:
    Timeline::TimelineWidget* m_timeline = nullptr;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序属性
    app.setApplicationName(QStringLiteral("Timeline Demo"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("VLayout"));

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
