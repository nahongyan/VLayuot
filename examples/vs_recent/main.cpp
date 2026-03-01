/**
 * @file main.cpp
 * @brief VS 最近项目列表示例程序入口
 *
 * 演示如何使用 VLayout 框架实现 Visual Studio 2022 风格的最近项目列表。
 */

#include "recentprojectswindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName(QStringLiteral("VS Recent Projects Demo"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("VLayout"));

    // 创建并显示主窗口
    VS::RecentProjectsWindow window;
    window.show();

    return app.exec();
}
