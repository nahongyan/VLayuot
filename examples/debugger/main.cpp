/**
 * VLayout 调试器示例
 *
 * 演示如何使用 LayoutSandboxWidget 调试布局。
 */

#include <QApplication>
#include <vlayout/debugger/sandbox_widget.h>

using namespace VLayout;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 创建沙盒窗口
    auto sandbox = new LayoutSandboxWidget();
    sandbox->setWindowTitle("VLayout 布局调试器");

    // 设置容器和布局参数
    sandbox->setContainerSize(400, 200);
    sandbox->setMargins(16, 8, 16, 8);
    sandbox->setSpacing(8);
    sandbox->setLayoutDirection(BoxLayout::Direction::LeftToRight);

    // 添加布局项 - 模拟典型的列表项布局
    // [图标(36px)] [标题(stretch)] [徽章(24px)]
    sandbox->addFixedItem("icon", 36);
    sandbox->addStretchItem("title", 1, 100);
    sandbox->addFixedItem("badge", 24);

    sandbox->show();

    return app.exec();
}
