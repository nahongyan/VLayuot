/**
 * @file tst_delegate.cpp
 * @brief VLayout 单元测试入口
 *
 * 包含所有测试类的运行入口
 */

#include <QtTest/QtTest>
#include <QApplication>
#include <vlayout/framework.h>

using namespace VLayout;

// ============================================================================
// TestDelegate - Delegate 和组件测试
// ============================================================================

class TestDelegate : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testComponentCreation();
    void testBinding();
    void testLayout();
};

void TestDelegate::initTestCase()
{
    // 测试初始化
}

void TestDelegate::cleanupTestCase()
{
    // 测试清理
}

void TestDelegate::testComponentCreation()
{
    // 测试组件创建
    LabelComponent label("testLabel");
    QCOMPARE(label.id(), QString("testLabel"));
}

void TestDelegate::testBinding()
{
    // TODO: 添加绑定测试
}

void TestDelegate::testLayout()
{
    // TODO: 添加布局测试
}

// ============================================================================
// 主入口 - 运行所有测试
// ============================================================================

// 前向声明（在 tst_boxlayout.cpp 中定义）
class TestBoxLayout;

// 外部函数声明
extern int runBoxLayoutTests(int argc, char *argv[]);
extern int runSandboxPreviewTests(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int result = 0;

    // 运行 Delegate 测试
    {
        TestDelegate td;
        result |= QTest::qExec(&td, argc, argv);
    }

    // 运行 BoxLayout 测试
    result |= runBoxLayoutTests(argc, argv);

    // 运行 SandboxPreview 测试
    result |= runSandboxPreviewTests(argc, argv);

    return result;
}

#include "tst_delegate.moc"
