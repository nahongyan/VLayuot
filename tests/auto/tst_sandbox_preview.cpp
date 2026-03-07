/**
 * @file tst_sandbox_preview.cpp
 * @brief SandboxPreview 单元测试
 *
 * 测试 SandboxPreview 使用真实 BoxLayout 的行为
 *
 * 重要发现：BoxLayout 交叉方向尺寸规则（与 Qt QBoxLayout 一致）
 * - 默认行为：交叉方向填充整个容器
 * - 设置对齐后：交叉方向使用 sizeHint
 * - maxSize 始终有效：即使填充容器，也不会超过 maxSize
 */

#include <QtTest/QtTest>
#include <vlayout/debugger/sandbox_preview.h>
#include <vlayout/boxlayout.h>
#include <vlayout/widgetitem.h>
#include <vlayout/spaceritem.h>

using namespace VLayout;

class TestSandboxPreview : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // ========== 基础功能测试 ==========
    void testEmptyLayout();
    void testSingleItem();

    // ========== 交叉尺寸行为测试 ==========
    // 当前 BoxLayout 行为：无对齐时填充容器，有对齐时使用 sizeHint

    void testHorizontal_FillsContainer_WithoutAlignment();
    void testHorizontal_UsesSizeHint_WithAlignment();
    void testHorizontal_RespectsMaxSize_WhenFillingContainer();
    void testHorizontal_RespectsMinSize_WithAlignment();
    void testHorizontal_RespectsMaxSize_WithAlignment();

    // 垂直布局
    void testVertical_FillsContainer_WithoutAlignment();
    void testVertical_UsesSizeHint_WithAlignment();

    // ========== 嵌套布局测试 ==========
    void testNestedLayout();

private:
    SandboxPreview* m_preview = nullptr;
};

void TestSandboxPreview::initTestCase()
{
    m_preview = new SandboxPreview();
}

void TestSandboxPreview::cleanupTestCase()
{
    delete m_preview;
    m_preview = nullptr;
}

void TestSandboxPreview::testEmptyLayout()
{
    auto layout = m_preview->layout();
    QVERIFY(layout != nullptr);

    layout->clear();
    m_preview->setContainerSize(400, 200);
    m_preview->computeLayout();

    QCOMPARE(layout->count(), 0);
    QVERIFY(m_preview->items().empty());
}

void TestSandboxPreview::testSingleItem()
{
    auto layout = m_preview->layout();
    layout->clear();

    auto item = createButton("btn1", "Button");
    item->setFixedSize(QSize(100, 40));
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->computeLayout();

    QCOMPARE(layout->count(), 1);
    QCOMPARE(m_preview->items().size(), static_cast<size_t>(1));

    // 检查计算结果
    const auto& result = m_preview->items()[0];
    QCOMPARE(result.id, QString("btn1"));
    QCOMPARE(result.size, 100);  // 水平布局，主方向尺寸

    layout->clear();
}

// ============================================================================
// 水平布局交叉尺寸测试
// ============================================================================

void TestSandboxPreview::testHorizontal_FillsContainer_WithoutAlignment()
{
    // 场景：水平布局，没有设置对齐
    // 期望：交叉方向（高度）填充整个容器（200px）

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::LeftToRight);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(100, 50));      // sizeHint 高度 50
    item->setMinimumSize(QSize(0, 0));
    item->setMaximumSize(QSize(1000, 1000000)); // 允许很大
    // 注意：没有设置对齐！
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== 无对齐时行为 ===";
    qDebug() << "Container height: 200, sizeHint.height: 50";
    qDebug() << "Actual crossSize:" << result.crossSize;
    qDebug() << "结论：无对齐时，交叉方向填充容器";

    // 无对齐时，高度应该填充容器（200px）
    QCOMPARE(result.crossSize, 200);

    layout->clear();
}

void TestSandboxPreview::testHorizontal_UsesSizeHint_WithAlignment()
{
    // 场景：水平布局，设置了垂直对齐
    // 期望：交叉方向（高度）使用 sizeHint

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::LeftToRight);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(100, 50));      // sizeHint 高度 50
    item->setMinimumSize(QSize(0, 30));
    item->setMaximumSize(QSize(1000, 100));
    item->setAlignment(Qt::AlignVCenter);   // 设置垂直对齐！
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== 有对齐时行为 ===";
    qDebug() << "Container height: 200, sizeHint.height: 50, Alignment: VCenter";
    qDebug() << "Actual crossSize:" << result.crossSize;
    qDebug() << "结论：有对齐时，交叉方向使用 sizeHint";

    // 有对齐时，高度应该使用 sizeHint（50px）
    QCOMPARE(result.crossSize, 50);

    layout->clear();
}

void TestSandboxPreview::testHorizontal_RespectsMaxSize_WhenFillingContainer()
{
    // 场景：无对齐，但 maxSize 限制交叉方向最大值
    // 期望：即使填充容器，也不超过 maxSize

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::LeftToRight);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(100, 50));
    item->setMinimumSize(QSize(0, 0));
    item->setMaximumSize(QSize(1000, 60));  // 最大高度 60
    // 没有设置对齐
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== maxSize 限制测试 ===";
    qDebug() << "Container height: 200, maxSize.height: 60, No alignment";
    qDebug() << "Actual crossSize:" << result.crossSize;
    qDebug() << "结论：maxSize 始终有效";

    // 即使无对齐想填充容器(200)，也会被 maxSize(60) 限制
    QCOMPARE(result.crossSize, 60);

    layout->clear();
}

void TestSandboxPreview::testHorizontal_RespectsMinSize_WithAlignment()
{
    // 场景：有对齐，sizeHint < minSize
    // 期望：交叉方向使用 minSize 而不是 sizeHint

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::LeftToRight);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(100, 30));     // sizeHint 高度 30
    item->setMinimumSize(QSize(0, 50));    // minSize 高度 50
    item->setMaximumSize(QSize(1000, 100));
    item->setAlignment(Qt::AlignVCenter);  // 设置对齐
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== minSize 限制测试（有对齐）===";
    qDebug() << "sizeHint.height: 30, minSize.height: 50";
    qDebug() << "Actual crossSize:" << result.crossSize;

    // 应该使用 minSize（50）而不是 sizeHint（30）
    QCOMPARE(result.crossSize, 50);

    layout->clear();
}

void TestSandboxPreview::testHorizontal_RespectsMaxSize_WithAlignment()
{
    // 场景：有对齐，sizeHint > maxSize
    // 期望：交叉方向使用 maxSize 而不是 sizeHint

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::LeftToRight);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(100, 150));    // sizeHint 高度 150
    item->setMinimumSize(QSize(0, 0));
    item->setMaximumSize(QSize(1000, 60)); // maxSize 高度 60
    item->setAlignment(Qt::AlignVCenter);
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== maxSize 限制测试（有对齐）===";
    qDebug() << "sizeHint.height: 150, maxSize.height: 60";
    qDebug() << "Actual crossSize:" << result.crossSize;

    // 应该使用 maxSize（60）而不是 sizeHint（150）
    QCOMPARE(result.crossSize, 60);

    layout->clear();
}

// ============================================================================
// 垂直布局交叉尺寸测试
// ============================================================================

void TestSandboxPreview::testVertical_FillsContainer_WithoutAlignment()
{
    // 场景：垂直布局，没有设置对齐
    // 期望：交叉方向（宽度）填充整个容器（400px）

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::TopToBottom);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(150, 40));     // sizeHint 宽度 150
    // 没有设置对齐
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== 垂直布局无对齐 ===";
    qDebug() << "Container width: 400, sizeHint.width: 150";
    qDebug() << "Actual crossSize:" << result.crossSize;

    QCOMPARE(result.crossSize, 400);

    layout->clear();
}

void TestSandboxPreview::testVertical_UsesSizeHint_WithAlignment()
{
    // 场景：垂直布局，设置了水平对齐
    // 期望：交叉方向（宽度）使用 sizeHint

    auto layout = m_preview->layout();
    layout->clear();
    layout->setDirection(BoxLayout::Direction::TopToBottom);

    auto item = createButton("btn1", "Button");
    item->setSizeHint(QSize(150, 40));     // sizeHint 宽度 150
    item->setAlignment(Qt::AlignHCenter);  // 设置水平对齐
    layout->addItem(item);

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(0, 0, 0, 0);
    m_preview->computeLayout();

    const auto& result = m_preview->items()[0];

    qDebug() << "=== 垂直布局有对齐 ===";
    qDebug() << "Container width: 400, sizeHint.width: 150, Alignment: HCenter";
    qDebug() << "Actual crossSize:" << result.crossSize;

    QCOMPARE(result.crossSize, 150);

    layout->clear();
}

// ============================================================================
// 嵌套布局测试
// ============================================================================

void TestSandboxPreview::testNestedLayout()
{
    auto outerLayout = m_preview->layout();
    outerLayout->clear();
    outerLayout->setDirection(BoxLayout::Direction::TopToBottom);

    // 内层水平布局
    auto innerLayout = std::make_shared<HBoxLayout>();
    innerLayout->addItem(createLabel("label1", "Name:"));
    innerLayout->addItem(createLineEdit("edit1"));
    innerLayout->addItem(createStretch());

    // 添加到外层
    outerLayout->addItem(innerLayout);
    outerLayout->addItem(createButton("submit", "Submit"));

    m_preview->setContainerSize(400, 200);
    m_preview->setMargins(10, 10, 10, 10);
    m_preview->computeLayout();

    QCOMPARE(outerLayout->count(), 2);

    auto inner = std::dynamic_pointer_cast<BoxLayout>(outerLayout->itemAt(0));
    QVERIFY(inner != nullptr);
    QCOMPARE(inner->count(), 3);

    qDebug() << "Nested layout test passed";
    qDebug() << "Outer layout items:" << outerLayout->count();
    qDebug() << "Inner layout items:" << inner->count();

    outerLayout->clear();
}

int runSandboxPreviewTests(int argc, char *argv[])
{
    TestSandboxPreview t;
    return QTest::qExec(&t, argc, argv);
}

#include "tst_sandbox_preview.moc"
