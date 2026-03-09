/**
 * VLayout 布局调试工具
 *
 * 功能：
 * - 可视化调试布局
 * - 测试布局效果
 * - 生成 Delegate 代码 (TODO)
 * - 生成 Model 代码 (TODO)
 */

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include "sandbox_widget.h"

using namespace VLayout;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("VLayout Debugger");
    app.setApplicationVersion("1.0.0");

    // 创建主窗口
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("VLayout 布局调试器");
    mainWindow.resize(800, 600);

    // 创建沙盒窗口作为中心部件
    auto sandbox = new LayoutSandboxWidget(&mainWindow);
    mainWindow.setCentralWidget(sandbox);

    // 设置默认布局参数
    sandbox->setContainerSize(400, 200);
    sandbox->setMargins(16, 8, 16, 8);
    sandbox->setSpacing(8);
    sandbox->setLayoutDirection(BoxLayout::Direction::LeftToRight);

    // 添加默认布局项
    sandbox->addFixedItem("icon", 36);
    sandbox->addStretchItem("title", 1, 100);
    sandbox->addFixedItem("badge", 24);

    // 创建菜单
    auto fileMenu = mainWindow.menuBar()->addMenu("文件");
    fileMenu->addAction("退出", &app, &QApplication::quit);

    auto helpMenu = mainWindow.menuBar()->addMenu("帮助");
    helpMenu->addAction("关于", [&]() {
        QMessageBox::about(&mainWindow, "关于",
            "VLayout 布局调试工具\n\n"
            "可视化调试布局效果\n"
            "测试各种布局配置");
    });

    mainWindow.show();

    return app.exec();
}
