/**
 * @file tst_boxlayout.cpp
 * @brief BoxLayout 单元测试 - 覆盖所有布局场景
 */

#include <QtTest/QtTest>
#include <vlayout/boxlayout.h>
#include <vlayout/widgetitem.h>
#include <vlayout/spaceritem.h>

using namespace VLayout;

class TestBoxLayout : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // ========== B: 基础布局测试 ==========
    void B01_emptyLayout();
    void B02_singleItem();
    void B03_twoItemsH();
    void B04_twoItemsV();
    void B05_threeItemsH();
    void B06_threeItemsV();
    void B07_manyItems();

    // ========== M: 边距测试 ==========
    void M01_uniformMargins();
    void M02_differentMargins();
    void M03_zeroMargins();
    void M04_largeMargins();
    void M05_marginsExceedSpace();
    void M06_negativeMargins();
    void M07_marginsChange();

    // ========== S: 间距测试 ==========
    void S01_defaultSpacing();
    void S02_positiveSpacing();
    void S03_largeSpacing();
    void S04_zeroSpacing();
    void S05_spacingChange();
    void S06_spacingWithHiddenItem();
    void S07_spacingAtEdges();

    // ========== T: Stretch 分配测试 ==========
    void T01_noStretch();
    void T02_equalStretch();
    void T03_differentStretch();
    void T04_oneStretch();
    void T05_allStretchZero();
    void T06_largeStretch();
    void T07_stretchWithFixedSize();
    void T08_stretchUpdatesOnItemChange();

    // ========== C: 尺寸约束测试 ==========
    void C01_fixedSize();
    void C02_minimumSize();
    void C03_maximumSize();
    void C04_mixedConstraints();
    void C05_allFixed();
    void C06_minGreaterThanHint();
    void C07_maxLessThanHint();
    void C08_zeroMinSize();
    void C09_hugeMaxSize();
    void C10_constraintConflicts();

    // ========== E: 边界场景测试 ==========
    void E01_zeroSpace();
    void E02_onePixelSpace();
    void E03_negativeSpace();
    void E04_spaceLessThanMin();
    void E05_spaceEqualsMin();
    void E06_spaceEqualsHint();
    void E07_spaceGreaterThanMax();
    void E08_emptyItem();
    void E09_allEmptyItems();
    void E10_singlePixelItems();
    void E11_hugeItems();
    void E12_oddPixelDistribution();

    // ========== A: 对齐测试 ==========
    void A01_alignLeft();
    void A02_alignRight();
    void A03_alignHCenter();
    void A04_alignTop();
    void A05_alignBottom();
    void A06_alignVCenter();
    void A07_alignCenter();
    void A08_noAlignment();
    void A09_mixedAlignment();

    // ========== N: 嵌套布局测试 ==========
    void N01_hboxInVbox();
    void N02_vboxInHbox();
    void N03_deepNesting();
    void N04_nestedMargins();
    void N05_nestedStretch();
    void N06_mixedItemsAndLayouts();

    // ========== D: 动态变化测试 ==========
    void D01_addItem();
    void D02_removeItem();
    void D03_insertItem();
    void D04_clearItems();
    void D05_changeDirection();
    void D06_itemSizeChange();
    void D07_multipleChanges();
    void D08_invalidate();

    // ========== R: 尺寸协商测试 ==========
    void R01_sizeHintCalculation();
    void R02_minimumSizeCalculation();
    void R03_maximumSizeCalculation();
    void R04_expandingDirections();
    void R05_sizeHintWithMargins();
    void R06_sizeHintWithSpacing();

    // ========== P: SpacerItem 专项测试 ==========
    void P01_fixedSpacer();
    void P02_expandingSpacer();
    void P03_multipleSpacers();
    void P04_spacerWithStretch();
    void P05_spacerBetweenWidgets();

    // ========== H: 便捷类测试 ==========
    void H01_hBoxLayout();
    void H02_vBoxLayout();

private:
    std::shared_ptr<WidgetItem> createWidget(const QString& id, int w, int h);
    std::shared_ptr<WidgetItem> createWidget(const QString& id, int w, int h, int minW, int minH, int maxW, int maxH);
};

std::shared_ptr<WidgetItem> TestBoxLayout::createWidget(const QString& id, int w, int h)
{
    auto item = std::make_shared<WidgetItem>(id);
    item->setSizeHint(QSize(w, h));
    return item;
}

std::shared_ptr<WidgetItem> TestBoxLayout::createWidget(const QString& id, int w, int h, int minW, int minH, int maxW, int maxH)
{
    auto item = std::make_shared<WidgetItem>(id);
    item->setSizeHint(QSize(w, h));
    item->setMinimumSize(QSize(minW, minH));
    item->setMaximumSize(QSize(maxW, maxH));
    return item;
}

void TestBoxLayout::initTestCase() {}
void TestBoxLayout::cleanupTestCase() {}

// ============================================================================
// B: 基础布局测试
// ============================================================================

void TestBoxLayout::B01_emptyLayout()
{
    auto layout = std::make_shared<HBoxLayout>();
    QCOMPARE(layout->sizeHint(), QSize(0, 0));
    QCOMPARE(layout->minimumSize(), QSize(0, 0));
    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();
    QVERIFY(layout->isValid());
}

void TestBoxLayout::B02_singleItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    QVERIFY(item->finalRect().x() >= 0);
    QVERIFY(item->finalRect().width() > 0);
    QCOMPARE(item->finalRect().height(), 50);
}

void TestBoxLayout::B03_twoItemsH()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 80, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    QVERIFY(item2->finalRect().x() > item1->finalRect().x());
    QCOMPARE(item1->finalRect().height(), 50);
    QCOMPARE(item2->finalRect().height(), 50);
    QCOMPARE(item1->finalRect().width() + item2->finalRect().width(), 200);
}

void TestBoxLayout::B04_twoItemsV()
{
    auto layout = std::make_shared<VBoxLayout>();
    auto item1 = createWidget("item1", 100, 30);
    auto item2 = createWidget("item2", 100, 40);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 150, 100));
    layout->activate();

    QCOMPARE(item1->finalRect().y(), 0);
    QVERIFY(item2->finalRect().y() > item1->finalRect().y());
    QCOMPARE(item1->finalRect().width(), 150);
    QCOMPARE(item2->finalRect().width(), 150);
    QCOMPARE(item1->finalRect().height() + item2->finalRect().height(), 100);
}

void TestBoxLayout::B05_threeItemsH()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    auto item3 = createWidget("item3", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);
    layout->addItem(item3);

    layout->setGeometry(QRect(0, 0, 300, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    QVERIFY(item2->finalRect().x() >= item1->finalRect().right() - 1);
    QVERIFY(item3->finalRect().x() >= item2->finalRect().right() - 1);
    QCOMPARE(item1->finalRect().width() + item2->finalRect().width() + item3->finalRect().width(), 300);
}

void TestBoxLayout::B06_threeItemsV()
{
    auto layout = std::make_shared<VBoxLayout>();
    auto item1 = createWidget("item1", 100, 30);
    auto item2 = createWidget("item2", 100, 30);
    auto item3 = createWidget("item3", 100, 30);
    layout->addItem(item1);
    layout->addItem(item2);
    layout->addItem(item3);

    layout->setGeometry(QRect(0, 0, 100, 90));
    layout->activate();

    QCOMPARE(item1->finalRect().y(), 0);
    QVERIFY(item2->finalRect().y() >= item1->finalRect().bottom() - 1);
    QVERIFY(item3->finalRect().y() >= item2->finalRect().bottom() - 1);
    QCOMPARE(item1->finalRect().height() + item2->finalRect().height() + item3->finalRect().height(), 90);
}

void TestBoxLayout::B07_manyItems()
{
    auto layout = std::make_shared<HBoxLayout>();
    const int itemCount = 20;
    std::vector<std::shared_ptr<WidgetItem>> items;

    for (int i = 0; i < itemCount; ++i) {
        auto item = createWidget(QString("item%1").arg(i), 50, 30);
        items.push_back(item);
        layout->addItem(item);
    }

    layout->setGeometry(QRect(0, 0, 1000, 40));
    layout->activate();

    int totalWidth = 0;
    for (int i = 0; i < itemCount; ++i) {
        QVERIFY(items[i]->finalRect().width() > 0);
        QCOMPARE(items[i]->finalRect().height(), 40);
        totalWidth += items[i]->finalRect().width();
    }
    QCOMPARE(totalWidth, 1000);
}

// ============================================================================
// M: 边距测试
// ============================================================================

void TestBoxLayout::M01_uniformMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(10);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 60));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 10);
    QCOMPARE(item->finalRect().y(), 10);
    QCOMPARE(item->finalRect().width(), 180);
    QCOMPARE(item->finalRect().height(), 40);
}

void TestBoxLayout::M02_differentMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(10, 20, 30, 40);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 100));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 10);
    QCOMPARE(item->finalRect().y(), 20);
    QCOMPARE(item->finalRect().width(), 160);
    QCOMPARE(item->finalRect().height(), 40);
}

void TestBoxLayout::M03_zeroMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(0, 0, 0, 0);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 0);
    QCOMPARE(item->finalRect().y(), 0);
    QCOMPARE(item->finalRect().height(), 50);
}

void TestBoxLayout::M04_largeMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(50, 50, 50, 50);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 150, 150));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 50);
    QCOMPARE(item->finalRect().y(), 50);
    QCOMPARE(item->finalRect().width(), 50);
    QCOMPARE(item->finalRect().height(), 50);
}

void TestBoxLayout::M05_marginsExceedSpace()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(100, 100, 100, 100);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 150, 150));
    layout->activate();

    QVERIFY(item->finalRect().x() >= 0);
    QVERIFY(item->finalRect().y() >= 0);
}

void TestBoxLayout::M06_negativeMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(-10, -10, -10, -10);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QVERIFY(true); // 主要确保不崩溃
}

void TestBoxLayout::M07_marginsChange()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(10, 10, 10, 10);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 60));
    layout->activate();
    QCOMPARE(item->finalRect().x(), 10);
    QCOMPARE(item->finalRect().width(), 180);

    layout->setContentsMargins(20, 20, 20, 20);
    layout->activate();
    QCOMPARE(item->finalRect().x(), 20);
    QCOMPARE(item->finalRect().width(), 160);
}

// ============================================================================
// S: 间距测试
// ============================================================================

void TestBoxLayout::S01_defaultSpacing()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    QCOMPARE(item1->finalRect().width() + item2->finalRect().width(), 100);
}

void TestBoxLayout::S02_positiveSpacing()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(8);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 108, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    QVERIFY(item2->finalRect().x() > item1->finalRect().width());
}

void TestBoxLayout::S03_largeSpacing()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(100);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 150, 40));
    layout->activate();

    QVERIFY(item1->finalRect().x() == 0);
    QVERIFY(item2->finalRect().x() >= item1->finalRect().right());
}

void TestBoxLayout::S04_zeroSpacing()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(0);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    // 两项总宽度应等于100
    QCOMPARE(item1->finalRect().width() + item2->finalRect().width(), 100);
}

void TestBoxLayout::S05_spacingChange()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(8);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 108, 40));
    layout->activate();
    QVERIFY(item2->finalRect().x() > 50);

    layout->setSpacing(16);
    layout->activate();
    QVERIFY(item2->finalRect().x() > 50);
}

void TestBoxLayout::S06_spacingWithHiddenItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(10);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    auto item3 = createWidget("item3", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);
    layout->addItem(item3);

    item2->setVisible(false);

    layout->setGeometry(QRect(0, 0, 120, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().x(), 0);
    QVERIFY(item3->finalRect().x() > item1->finalRect().right());
}

void TestBoxLayout::S07_spacingAtEdges()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 0);
    QCOMPARE(item->finalRect().width(), 100);
}

// ============================================================================
// T: Stretch 分配测试
// ============================================================================

void TestBoxLayout::T01_noStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setStretch(0);
    item2->setStretch(0);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 100);
    QCOMPARE(item2->finalRect().width(), 100);
}

void TestBoxLayout::T02_equalStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setStretch(1);
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 100);
    QCOMPARE(item2->finalRect().width(), 100);
}

void TestBoxLayout::T03_differentStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 0, 30);
    auto item2 = createWidget("item2", 0, 30);
    item1->setStretch(1);
    item2->setStretch(3);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 50);
    QCOMPARE(item2->finalRect().width(), 150);
}

void TestBoxLayout::T04_oneStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setStretch(0);
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 50);
    QCOMPARE(item2->finalRect().width(), 150);
}

void TestBoxLayout::T05_allStretchZero()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setStretch(0);
    item2->setStretch(0);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 100);
    QCOMPARE(item2->finalRect().width(), 100);
}

void TestBoxLayout::T06_largeStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 0, 30);
    auto item2 = createWidget("item2", 0, 30);
    item1->setStretch(100);
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 101, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 100);
    QCOMPARE(item2->finalRect().width(), 1);
}

void TestBoxLayout::T07_stretchWithFixedSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setFixedSize(QSize(50, 30));
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 50);
    QCOMPARE(item2->finalRect().width(), 150);
}

void TestBoxLayout::T08_stretchUpdatesOnItemChange()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    item1->setStretch(1);
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();
    QCOMPARE(item1->finalRect().width(), 100);

    item1->setStretch(3);
    layout->invalidate();
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 150);
    QCOMPARE(item2->finalRect().width(), 50);
}

// ============================================================================
// C: 尺寸约束测试
// ============================================================================

void TestBoxLayout::C01_fixedSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 100, 30, 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    // 水平方向固定宽度生效
    QCOMPARE(item->finalRect().width(), 100);
    // 垂直方向在 HBoxLayout 中会填满可用高度（除非设置了 alignment）
    QVERIFY(item->finalRect().height() > 0);
}

void TestBoxLayout::C02_minimumSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 80, 20, SizeMax, SizeMax);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 50, 10));
    layout->activate();

    // 宽度约束生效，高度在 HBoxLayout 中会填满
    QVERIFY(item->finalRect().width() >= 80);
    QVERIFY(item->finalRect().height() > 0);
}

void TestBoxLayout::C03_maximumSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 0, 0, 120, 40);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    // 宽度约束生效，高度在 HBoxLayout 中可能填满
    QVERIFY(item->finalRect().width() <= 120);
    QVERIFY(item->finalRect().height() > 0);
}

void TestBoxLayout::C04_mixedConstraints()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto fixed = createWidget("fixed", 50, 30, 50, 30, 50, 30);
    auto flexible = createWidget("flexible", 50, 30, 0, 0, SizeMax, SizeMax);
    flexible->setStretch(1);

    layout->addItem(fixed);
    layout->addItem(flexible);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(fixed->finalRect().width(), 50);
    QCOMPARE(flexible->finalRect().width(), 150);
}

void TestBoxLayout::C05_allFixed()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30, 50, 30, 50, 30);
    auto item2 = createWidget("item2", 50, 30, 50, 30, 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 80, 40));
    layout->activate();

    QVERIFY(item1->finalRect().width() > 0);
    QVERIFY(item2->finalRect().width() > 0);
}

void TestBoxLayout::C06_minGreaterThanHint()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20, 100, 30, SizeMax, SizeMax);
    layout->addItem(item);

    QCOMPARE(layout->minimumSize(), QSize(100, 30));

    layout->setGeometry(QRect(0, 0, 150, 50));
    layout->activate();

    QVERIFY(item->finalRect().width() >= 100);
    QVERIFY(item->finalRect().height() >= 30);
}

void TestBoxLayout::C07_maxLessThanHint()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 0, 0, 50, 20);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    // 宽度最大约束生效
    QVERIFY(item->finalRect().width() <= 50);
    // 高度在 HBoxLayout 中会填满可用空间
    QVERIFY(item->finalRect().height() > 0);
}

void TestBoxLayout::C08_zeroMinSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 0, 0, SizeMax, SizeMax);
    layout->addItem(item);

    QCOMPARE(layout->minimumSize(), QSize(0, 0));

    layout->setGeometry(QRect(0, 0, 50, 20));
    layout->activate();

    QVERIFY(item->finalRect().width() >= 0);
}

void TestBoxLayout::C09_hugeMaxSize()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 0, 0, SizeMax, SizeMax);
    layout->addItem(item);

    QCOMPARE(layout->maximumSize().width(), SizeMax);
}

void TestBoxLayout::C10_constraintConflicts()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 100, 30, 100, 30, 100, 30);
    auto item2 = createWidget("item2", 100, 30, 100, 30, 100, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QVERIFY(item1->finalRect().width() > 0);
    QVERIFY(item2->finalRect().width() > 0);
}

// ============================================================================
// E: 边界场景测试
// ============================================================================

void TestBoxLayout::E01_zeroSpace()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 0, 0));
    layout->activate();

    QVERIFY(true);
}

void TestBoxLayout::E02_onePixelSpace()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 1, 1));
    layout->activate();

    QVERIFY(item->finalRect().width() >= 0);
}

void TestBoxLayout::E03_negativeSpace()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, -10, -10));
    layout->activate();

    QVERIFY(true);
}

void TestBoxLayout::E04_spaceLessThanMin()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30, 50, 20, SizeMax, SizeMax);
    auto item2 = createWidget("item2", 50, 30, 50, 20, SizeMax, SizeMax);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 60, 40));
    layout->activate();

    QVERIFY(item1->finalRect().width() > 0);
    QVERIFY(item2->finalRect().width() > 0);
}

void TestBoxLayout::E05_spaceEqualsMin()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30, 40, 20, SizeMax, SizeMax);
    auto item2 = createWidget("item2", 50, 30, 40, 20, SizeMax, SizeMax);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 80, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 40);
    QCOMPARE(item2->finalRect().width(), 40);
}

void TestBoxLayout::E06_spaceEqualsHint()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 50);
    QCOMPARE(item2->finalRect().width(), 50);
}

void TestBoxLayout::E07_spaceGreaterThanMax()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 100, 30, 0, 0, 150, 40);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    // 水平方向的最大约束生效
    QVERIFY(item->finalRect().width() <= 150);
    // 垂直方向在 HBoxLayout 中会填满可用空间（除非设置了 alignment）
    QVERIFY(item->finalRect().height() > 0);
}

void TestBoxLayout::E08_emptyItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto emptyItem = createWidget("empty", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    emptyItem->setVisible(false);

    layout->addItem(item1);
    layout->addItem(emptyItem);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QVERIFY(true);
}

void TestBoxLayout::E09_allEmptyItems()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    item1->setVisible(false);
    item2->setVisible(false);

    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QVERIFY(true);
}

void TestBoxLayout::E10_singlePixelItems()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 1, 1);
    auto item2 = createWidget("item2", 1, 1);
    auto item3 = createWidget("item3", 1, 1);
    layout->addItem(item1);
    layout->addItem(item2);
    layout->addItem(item3);

    layout->setGeometry(QRect(0, 0, 3, 3));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 1);
    QCOMPARE(item2->finalRect().width(), 1);
    QCOMPARE(item3->finalRect().width(), 1);
}

void TestBoxLayout::E11_hugeItems()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 10000, 10000);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 500, 400));
    layout->activate();

    QVERIFY(item->finalRect().width() <= 500);
    QVERIFY(item->finalRect().height() <= 400);
}

void TestBoxLayout::E12_oddPixelDistribution()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 0, 30);
    auto item2 = createWidget("item2", 0, 30);
    item1->setStretch(1);
    item2->setStretch(1);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 101, 40));
    layout->activate();

    int totalWidth = item1->finalRect().width() + item2->finalRect().width();
    QCOMPARE(totalWidth, 101);
}

// ============================================================================
// A: 对齐测试
// ============================================================================

void TestBoxLayout::A01_alignLeft()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    item->setAlignment(Qt::AlignLeft);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 0);
}

void TestBoxLayout::A02_alignRight()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    item->setAlignment(Qt::AlignRight);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    // 右对齐：item 应在右边，宽度为 50
    QCOMPARE(item->finalRect().width(), 50);
    // 允许 1 像素误差（边界计算）
    QVERIFY(item->finalRect().right() >= 199);
}

void TestBoxLayout::A03_alignHCenter()
{
    auto layout = std::make_shared<VBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    item->setAlignment(Qt::AlignHCenter);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item->finalRect().x(), 75);
}

void TestBoxLayout::A04_alignTop()
{
    auto layout = std::make_shared<VBoxLayout>();
    auto item = createWidget("item1", 100, 20);
    item->setAlignment(Qt::AlignTop);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QCOMPARE(item->finalRect().y(), 0);
}

void TestBoxLayout::A05_alignBottom()
{
    auto layout = std::make_shared<VBoxLayout>();
    auto item = createWidget("item1", 100, 20);
    item->setAlignment(Qt::AlignBottom);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    // 底部对齐：item 应在底部，高度为 20
    QCOMPARE(item->finalRect().height(), 20);
    // 允许 1 像素误差
    QVERIFY(item->finalRect().bottom() >= 49);
}

void TestBoxLayout::A06_alignVCenter()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    item->setAlignment(Qt::AlignVCenter);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QCOMPARE(item->finalRect().y(), 15);
}

void TestBoxLayout::A07_alignCenter()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    item->setAlignment(Qt::AlignCenter);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QCOMPARE(item->finalRect().y(), 15);
}

void TestBoxLayout::A08_noAlignment()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item1", 50, 20);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QCOMPARE(item->finalRect().height(), 50);
}

void TestBoxLayout::A09_mixedAlignment()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 20);
    auto item2 = createWidget("item2", 50, 20);

    item1->setAlignment(Qt::AlignTop);
    item2->setAlignment(Qt::AlignBottom);

    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 50));
    layout->activate();

    QCOMPARE(item1->finalRect().y(), 0);
    // 允许 1 像素误差
    QVERIFY(item2->finalRect().bottom() >= 49);
}

// ============================================================================
// N: 嵌套布局测试
// ============================================================================

void TestBoxLayout::N01_hboxInVbox()
{
    auto outerLayout = std::make_shared<VBoxLayout>();
    auto innerLayout = std::make_shared<HBoxLayout>();

    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    innerLayout->addItem(item1);
    innerLayout->addItem(item2);
    outerLayout->addItem(innerLayout);

    outerLayout->setGeometry(QRect(0, 0, 100, 60));
    outerLayout->activate();

    QVERIFY(item1->finalRect().isValid());
    QVERIFY(item2->finalRect().isValid());
}

void TestBoxLayout::N02_vboxInHbox()
{
    auto outerLayout = std::make_shared<HBoxLayout>();
    auto innerLayout = std::make_shared<VBoxLayout>();

    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    innerLayout->addItem(item1);
    innerLayout->addItem(item2);
    outerLayout->addItem(innerLayout);

    outerLayout->setGeometry(QRect(0, 0, 100, 60));
    outerLayout->activate();

    QVERIFY(item1->finalRect().isValid());
    QVERIFY(item2->finalRect().isValid());
}

void TestBoxLayout::N03_deepNesting()
{
    auto layout1 = std::make_shared<VBoxLayout>();
    auto layout2 = std::make_shared<HBoxLayout>();
    auto layout3 = std::make_shared<VBoxLayout>();

    auto item = createWidget("item", 50, 30);
    layout3->addItem(item);
    layout2->addItem(layout3);
    layout1->addItem(layout2);

    layout1->setGeometry(QRect(0, 0, 100, 80));
    layout1->activate();

    QVERIFY(item->finalRect().isValid());
}

void TestBoxLayout::N04_nestedMargins()
{
    auto outerLayout = std::make_shared<HBoxLayout>();
    outerLayout->setContentsMargins(10, 10, 10, 10);

    auto innerLayout = std::make_shared<HBoxLayout>();
    innerLayout->setContentsMargins(5, 5, 5, 5);

    auto item = createWidget("item", 50, 30);
    innerLayout->addItem(item);
    outerLayout->addItem(innerLayout);

    outerLayout->setGeometry(QRect(0, 0, 100, 60));
    outerLayout->activate();

    QCOMPARE(item->finalRect().x(), 15);
}

void TestBoxLayout::N05_nestedStretch()
{
    auto outerLayout = std::make_shared<HBoxLayout>();
    auto innerLayout1 = std::make_shared<VBoxLayout>();
    auto innerLayout2 = std::make_shared<VBoxLayout>();

    innerLayout1->setStretch(1);
    innerLayout2->setStretch(2);

    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    innerLayout1->addItem(item1);
    innerLayout2->addItem(item2);

    outerLayout->addItem(innerLayout1);
    outerLayout->addItem(innerLayout2);

    outerLayout->setGeometry(QRect(0, 0, 150, 40));
    outerLayout->activate();

    QCOMPARE(innerLayout1->finalRect().width(), 50);
    QCOMPARE(innerLayout2->finalRect().width(), 100);
}

void TestBoxLayout::N06_mixedItemsAndLayouts()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto innerLayout = std::make_shared<VBoxLayout>();
    auto item2 = createWidget("item2", 50, 30);

    innerLayout->addItem(item2);
    layout->addItem(item1);
    layout->addItem(innerLayout);

    layout->setGeometry(QRect(0, 0, 150, 40));
    layout->activate();

    QVERIFY(item1->finalRect().isValid());
    QVERIFY(item2->finalRect().isValid());
}

// ============================================================================
// D: 动态变化测试
// ============================================================================

void TestBoxLayout::D01_addItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(layout->count(), 0);

    auto item = createWidget("item1", 50, 30);
    layout->addItem(item);
    layout->activate();

    QCOMPARE(layout->count(), 1);
    QVERIFY(item->finalRect().isValid());
}

void TestBoxLayout::D02_removeItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);

    layout->addItem(item1);
    layout->addItem(item2);
    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(layout->count(), 2);

    layout->removeItem(item1);
    layout->activate();

    QCOMPARE(layout->count(), 1);
    QCOMPARE(item2->finalRect().width(), 100);
}

void TestBoxLayout::D03_insertItem()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    auto item3 = createWidget("item3", 50, 30);

    layout->addItem(item1);
    layout->addItem(item3);
    layout->insertItem(1, item2);

    layout->setGeometry(QRect(0, 0, 150, 40));
    layout->activate();

    QCOMPARE(layout->indexOf(item1), 0);
    QCOMPARE(layout->indexOf(item2), 1);
    QCOMPARE(layout->indexOf(item3), 2);
}

void TestBoxLayout::D04_clearItems()
{
    auto layout = std::make_shared<HBoxLayout>();
    for (int i = 0; i < 5; ++i) {
        layout->addItem(createWidget(QString("item%1").arg(i), 50, 30));
    }

    QCOMPARE(layout->count(), 5);

    layout->clear();
    QCOMPARE(layout->count(), 0);
}

void TestBoxLayout::D05_changeDirection()
{
    auto layout = std::make_shared<BoxLayout>(BoxLayout::Direction::LeftToRight);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 80));
    layout->activate();

    QVERIFY(item1->finalRect().x() < item2->finalRect().x());

    layout->setDirection(BoxLayout::Direction::TopToBottom);
    layout->activate();

    QVERIFY(item1->finalRect().y() < item2->finalRect().y());
}

void TestBoxLayout::D06_itemSizeChange()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item", 50, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();
    QCOMPARE(item->finalRect().width(), 100);

    item->setSizeHint(QSize(80, 30));
    layout->invalidate();
    layout->activate();

    QCOMPARE(item->finalRect().width(), 100);
}

void TestBoxLayout::D07_multipleChanges()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);

    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 50));
    layout->activate();

    for (int i = 0; i < 5; ++i) {
        layout->setSpacing(i * 2);
        layout->activate();
    }

    QVERIFY(true);
}

void TestBoxLayout::D08_invalidate()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item", 50, 30);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();
    QVERIFY(layout->isValid());

    layout->invalidate();
    QVERIFY(!layout->isValid());

    layout->activate();
    QVERIFY(layout->isValid());
}

// ============================================================================
// R: 尺寸协商测试
// ============================================================================

void TestBoxLayout::R01_sizeHintCalculation()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 80, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    QCOMPARE(layout->sizeHint(), QSize(130, 30));
}

void TestBoxLayout::R02_minimumSizeCalculation()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30, 30, 20, SizeMax, SizeMax);
    auto item2 = createWidget("item2", 80, 30, 40, 25, SizeMax, SizeMax);
    layout->addItem(item1);
    layout->addItem(item2);

    QCOMPARE(layout->minimumSize(), QSize(70, 25));
}

void TestBoxLayout::R03_maximumSizeCalculation()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30, 0, 0, 100, 40);
    auto item2 = createWidget("item2", 80, 30, 0, 0, 150, 50);
    layout->addItem(item1);
    layout->addItem(item2);

    // 水平方向：两个 item 的最大尺寸之和
    QCOMPARE(layout->maximumSize().width(), 250);
    // 垂直方向：取子项最大尺寸的最大值（如果子项有 SizeMax，结果可能是 SizeMax）
    QVERIFY(layout->maximumSize().height() >= 50);
}

void TestBoxLayout::R04_expandingDirections()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item = createWidget("item", 50, 30);
    item->setStretch(1);
    layout->addItem(item);

    QVERIFY(layout->expandingDirections() & Qt::Horizontal);
}

void TestBoxLayout::R05_sizeHintWithMargins()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setContentsMargins(10, 5, 10, 5);
    auto item = createWidget("item", 100, 30);
    layout->addItem(item);

    QCOMPARE(layout->sizeHint(), QSize(120, 40));
}

void TestBoxLayout::R06_sizeHintWithSpacing()
{
    auto layout = std::make_shared<HBoxLayout>();
    layout->setSpacing(8);
    auto item1 = createWidget("item1", 50, 30);
    auto item2 = createWidget("item2", 50, 30);
    layout->addItem(item1);
    layout->addItem(item2);

    QCOMPARE(layout->sizeHint(), QSize(108, 30));
}

// ============================================================================
// P: SpacerItem 专项测试
// ============================================================================

void TestBoxLayout::P01_fixedSpacer()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto spacer = createFixedSpacer(20, true);
    auto item = createWidget("item", 50, 30);

    layout->addItem(spacer);
    layout->addItem(item);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    QCOMPARE(spacer->finalRect().width(), 20);
}

void TestBoxLayout::P02_expandingSpacer()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 50, 30);
    auto spacer = createExpandingSpacer(1, true);
    auto item2 = createWidget("item2", 50, 30);

    layout->addItem(item1);
    layout->addItem(spacer);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 200, 40));
    layout->activate();

    QCOMPARE(item1->finalRect().width(), 50);
    QCOMPARE(item2->finalRect().width(), 50);
    QCOMPARE(spacer->finalRect().width(), 100);
}

void TestBoxLayout::P03_multipleSpacers()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto spacer1 = createFixedSpacer(10, true);
    auto item = createWidget("item", 50, 30);
    auto spacer2 = createExpandingSpacer(1, true);

    layout->addItem(spacer1);
    layout->addItem(item);
    layout->addItem(spacer2);

    layout->setGeometry(QRect(0, 0, 110, 40));
    layout->activate();

    QCOMPARE(spacer1->finalRect().width(), 10);
    QCOMPARE(item->finalRect().width(), 50);
}

void TestBoxLayout::P04_spacerWithStretch()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto spacer1 = createExpandingSpacer(1, true);
    auto spacer2 = createExpandingSpacer(2, true);

    layout->addItem(spacer1);
    layout->addItem(spacer2);

    layout->setGeometry(QRect(0, 0, 150, 40));
    layout->activate();

    QCOMPARE(spacer1->finalRect().width(), 50);
    QCOMPARE(spacer2->finalRect().width(), 100);
}

void TestBoxLayout::P05_spacerBetweenWidgets()
{
    auto layout = std::make_shared<HBoxLayout>();
    auto item1 = createWidget("item1", 40, 30);
    auto spacer = createFixedSpacer(20, true);
    auto item2 = createWidget("item2", 40, 30);

    layout->addItem(item1);
    layout->addItem(spacer);
    layout->addItem(item2);

    layout->setGeometry(QRect(0, 0, 100, 40));
    layout->activate();

    // item1 宽度为 40，从位置 0 开始
    // QRect::right() = x + width - 1 = 0 + 40 - 1 = 39
    QCOMPARE(item1->finalRect().width(), 40);
    QCOMPARE(item1->finalRect().x(), 0);
    // spacer 宽度为 20，从位置 40 开始
    QCOMPARE(spacer->finalRect().x(), 40);
    QCOMPARE(spacer->finalRect().width(), 20);
    // item2 从位置 60 开始
    QVERIFY(item2->finalRect().left() >= 60);
}

// ============================================================================
// H: 便捷类测试
// ============================================================================

void TestBoxLayout::H01_hBoxLayout()
{
    HBoxLayout layout;
    QCOMPARE(layout.direction(), BoxLayout::Direction::LeftToRight);
    QCOMPARE(layout.type(), ItemType::HBox);
}

void TestBoxLayout::H02_vBoxLayout()
{
    VBoxLayout layout;
    QCOMPARE(layout.direction(), BoxLayout::Direction::TopToBottom);
    QCOMPARE(layout.type(), ItemType::VBox);
}

// ============================================================================
// 外部入口函数
// ============================================================================

int runBoxLayoutTests(int argc, char *argv[])
{
    TestBoxLayout tb;
    return QTest::qExec(&tb, argc, argv);
}

#include "tst_boxlayout.moc"
