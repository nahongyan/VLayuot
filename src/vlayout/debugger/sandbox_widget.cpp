#include "sandbox_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QInputDialog>
#include <QMessageBox>

namespace VLayout {

// ============================================================================
// LayoutSandboxWidget 实现
// ============================================================================

LayoutSandboxWidget::LayoutSandboxWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    setupConnections();

    // 设置默认值
    setContainerSize(400, 200);
    setMargins(8, 8, 8, 8);
    setSpacing(8);

    // 设置窗口属性
    setWindowTitle(tr("VLayout 调试器"));
    resize(900, 600);
}

LayoutSandboxWidget::~LayoutSandboxWidget() = default;

// ========== UI 设置 ==========

void LayoutSandboxWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // ========== 参数面板 ==========

    auto* paramPanel = new QWidget;
    auto* paramLayout = new QHBoxLayout(paramPanel);
    paramLayout->setContentsMargins(0, 0, 0, 0);

    // 容器设置组
    auto* containerGroup = new QGroupBox(tr("容器设置"));
    auto* containerLayout = new QHBoxLayout(containerGroup);

    containerLayout->addWidget(new QLabel(tr("宽度:")));
    m_widthSpin = new QSpinBox;
    m_widthSpin->setRange(50, 2000);
    m_widthSpin->setValue(400);
    containerLayout->addWidget(m_widthSpin);
    containerLayout->addWidget(new QLabel(tr("px")));

    containerLayout->addSpacing(16);

    containerLayout->addWidget(new QLabel(tr("高度:")));
    m_heightSpin = new QSpinBox;
    m_heightSpin->setRange(50, 2000);
    m_heightSpin->setValue(200);
    containerLayout->addWidget(m_heightSpin);
    containerLayout->addWidget(new QLabel(tr("px")));

    containerLayout->addSpacing(16);

    containerLayout->addWidget(new QLabel(tr("方向:")));
    m_directionCombo = new QComboBox;
    m_directionCombo->addItem(tr("水平"), static_cast<int>(BoxLayout::Direction::LeftToRight));
    m_directionCombo->addItem(tr("垂直"), static_cast<int>(BoxLayout::Direction::TopToBottom));
    containerLayout->addWidget(m_directionCombo);

    containerLayout->addStretch();
    paramLayout->addWidget(containerGroup);

    // 布局设置组
    auto* layoutGroup = new QGroupBox(tr("布局设置"));
    auto* layoutSettingLayout = new QHBoxLayout(layoutGroup);

    layoutSettingLayout->addWidget(new QLabel(tr("Spacing:")));
    m_spacingSpin = new QSpinBox;
    m_spacingSpin->setRange(0, 100);
    m_spacingSpin->setValue(8);
    layoutSettingLayout->addWidget(m_spacingSpin);

    layoutSettingLayout->addSpacing(16);

    layoutSettingLayout->addWidget(new QLabel(tr("Margins:")));
    m_leftMarginSpin = new QSpinBox;
    m_leftMarginSpin->setRange(0, 100);
    m_leftMarginSpin->setValue(8);
    layoutSettingLayout->addWidget(m_leftMarginSpin);

    m_topMarginSpin = new QSpinBox;
    m_topMarginSpin->setRange(0, 100);
    m_topMarginSpin->setValue(8);
    layoutSettingLayout->addWidget(m_topMarginSpin);

    m_rightMarginSpin = new QSpinBox;
    m_rightMarginSpin->setRange(0, 100);
    m_rightMarginSpin->setValue(8);
    layoutSettingLayout->addWidget(m_rightMarginSpin);

    m_bottomMarginSpin = new QSpinBox;
    m_bottomMarginSpin->setRange(0, 100);
    m_bottomMarginSpin->setValue(8);
    layoutSettingLayout->addWidget(m_bottomMarginSpin);

    paramLayout->addWidget(layoutGroup);

    mainLayout->addWidget(paramPanel);

    // ========== 布局项表格 ==========

    auto* itemPanel = new QWidget;
    auto* itemPanelLayout = new QHBoxLayout(itemPanel);
    itemPanelLayout->setContentsMargins(0, 0, 0, 0);

    auto* itemGroup = new QGroupBox(tr("布局项"));
    auto* itemGroupLayout = new QVBoxLayout(itemGroup);

    m_itemTable = new QTableWidget;
    m_itemTable->setColumnCount(9);
    m_itemTable->setHorizontalHeaderLabels({
        tr("ID"), tr("Type"), tr("SizeHint"), tr("Stretch"), tr("Min"), tr("Max"),
        tr("CrossHint"), tr("CrossMin"), tr("CrossMax")
    });
    m_itemTable->horizontalHeader()->setStretchLastSection(true);
    m_itemTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_itemTable->setAlternatingRowColors(true);
    itemGroupLayout->addWidget(m_itemTable);

    // 添加/删除按钮
    auto* buttonLayout = new QHBoxLayout;
    auto* addButton = new QPushButton(tr("添加项"));
    auto* removeButton = new QPushButton(tr("删除项"));
    auto* clearButton = new QPushButton(tr("清空"));
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    itemGroupLayout->addLayout(buttonLayout);

    connect(addButton, &QPushButton::clicked, this, &LayoutSandboxWidget::onAddItemClicked);
    connect(removeButton, &QPushButton::clicked, this, &LayoutSandboxWidget::onRemoveItemClicked);
    connect(clearButton, &QPushButton::clicked, this, &LayoutSandboxWidget::clearItems);

    itemPanelLayout->addWidget(itemGroup);
    mainLayout->addWidget(itemPanel, 1);

    // ========== 下方面板（预览 + 结果） ==========

    m_verticalSplitter = new QSplitter(Qt::Vertical);

    // 预览区
    m_preview = new SandboxPreview;
    m_preview->setMinimumHeight(200);

    // 右侧面板：结果表格 + 诊断信息
    auto* rightPanel = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto* resultGroup = new QGroupBox(tr("计算结果"));
    auto* resultLayout = new QVBoxLayout(resultGroup);

    m_resultTable = new QTableWidget;
    m_resultTable->setColumnCount(9);
    m_resultTable->setHorizontalHeaderLabels({
        tr("ID"), tr("Hint"), tr("Min"), tr("Max"),
        tr("Stretch"), tr("Pos"), tr("Size"), tr("CrossSize"), tr("状态")
    });
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setAlternatingRowColors(true);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultLayout->addWidget(m_resultTable);

    rightLayout->addWidget(resultGroup);

    // 诊断信息
    auto* diagGroup = new QGroupBox(tr("诊断"));
    auto* diagLayout = new QVBoxLayout(diagGroup);
    m_diagnosticLabel = new QLabel;
    m_diagnosticLabel->setWordWrap(true);
    m_diagnosticLabel->setTextFormat(Qt::PlainText);
    diagLayout->addWidget(m_diagnosticLabel);
    rightLayout->addWidget(diagGroup);

    // 水平分割器
    m_horizontalSplitter = new QSplitter(Qt::Horizontal);
    m_horizontalSplitter->addWidget(m_preview);
    m_horizontalSplitter->addWidget(rightPanel);
    m_horizontalSplitter->setSizes({500, 400});

    m_verticalSplitter->addWidget(m_horizontalSplitter);
    mainLayout->addWidget(m_verticalSplitter, 2);
}

void LayoutSandboxWidget::setupConnections()
{
    // 容器设置变化
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onContainerSizeChanged);
    connect(m_heightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onContainerSizeChanged);
    connect(m_directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LayoutSandboxWidget::onContainerSizeChanged);

    // 布局参数变化
    connect(m_spacingSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onLayoutParamChanged);
    connect(m_leftMarginSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onLayoutParamChanged);
    connect(m_topMarginSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onLayoutParamChanged);
    connect(m_rightMarginSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onLayoutParamChanged);
    connect(m_bottomMarginSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LayoutSandboxWidget::onLayoutParamChanged);

    // 布局项表格变化
    connect(m_itemTable, &QTableWidget::cellChanged,
            this, &LayoutSandboxWidget::onItemTableCellChanged);

    // 预览区点击
    connect(m_preview, &SandboxPreview::itemClicked,
            this, &LayoutSandboxWidget::onPreviewItemClicked);

    // 布局计算完成
    connect(m_preview, &SandboxPreview::layoutComputed,
            this, &LayoutSandboxWidget::onLayoutComputed);
}

// ========== 容器设置 ==========

void LayoutSandboxWidget::setContainerSize(int width, int height)
{
    m_widthSpin->setValue(width);
    m_heightSpin->setValue(height);
    m_preview->setContainerSize(width, height);
}

QSize LayoutSandboxWidget::containerSize() const
{
    return QSize(m_widthSpin->value(), m_heightSpin->value());
}

void LayoutSandboxWidget::setLayoutDirection(BoxLayout::Direction direction)
{
    int index = (direction == BoxLayout::Direction::LeftToRight) ? 0 : 1;
    m_directionCombo->setCurrentIndex(index);
}

// ========== 布局参数 ==========

void LayoutSandboxWidget::setSpacing(int spacing)
{
    m_spacingSpin->setValue(spacing);
}

void LayoutSandboxWidget::setMargins(int left, int top, int right, int bottom)
{
    m_leftMarginSpin->setValue(left);
    m_topMarginSpin->setValue(top);
    m_rightMarginSpin->setValue(right);
    m_bottomMarginSpin->setValue(bottom);
}

void LayoutSandboxWidget::setMargins(int uniform)
{
    setMargins(uniform, uniform, uniform, uniform);
}

// ========== 布局项管理 ==========

void LayoutSandboxWidget::addFixedItem(const QString& id, int size, int crossSize)
{
    SandboxItem item;
    item.id = id;
    item.sizeHint = size;
    item.minSize = size;
    item.maxSize = size;
    item.stretch = 0;
    item.crossSizeHint = crossSize;
    item.crossMinSize = crossSize;
    item.crossMaxSize = crossSize;

    m_items.push_back(item);
    addItemToTable(item);
    updatePreview();
}

void LayoutSandboxWidget::addStretchItem(const QString& id, int stretch,
                                          int sizeHint, int minSize,
                                          int crossSizeHint, int crossMinSize)
{
    SandboxItem item;
    item.id = id;
    item.sizeHint = sizeHint;
    item.minSize = minSize;
    item.maxSize = 1000000;
    item.stretch = stretch;
    // 交叉方向
    item.crossSizeHint = crossSizeHint;
    item.crossMinSize = crossMinSize;
    item.crossMaxSize = 1000000;

    m_items.push_back(item);
    addItemToTable(item);
    updatePreview();
}

void LayoutSandboxWidget::addSpacing(int spacing)
{
    SandboxItem item;
    item.id = QString("spacing_%1").arg(m_items.size());
    item.sizeHint = spacing;
    item.minSize = spacing;
    item.maxSize = spacing;
    item.stretch = 0;
    item.isSpacing = true;

    m_items.push_back(item);
    addItemToTable(item);
    updatePreview();
}

void LayoutSandboxWidget::addCustomItem(const QString& id,
                                         int sizeHint, int minSize, int maxSize,
                                         int stretch,
                                         int crossSizeHint, int crossMinSize, int crossMaxSize)
{
    SandboxItem item;
    item.id = id;
    item.sizeHint = sizeHint;
    item.minSize = minSize;
    item.maxSize = maxSize;
    item.stretch = stretch;
    item.crossSizeHint = crossSizeHint;
    item.crossMinSize = crossMinSize;
    item.crossMaxSize = crossMaxSize;

    m_items.push_back(item);
    addItemToTable(item);
    updatePreview();
}

void LayoutSandboxWidget::clearItems()
{
    m_items.clear();
    m_itemTable->setRowCount(0);
    m_resultTable->setRowCount(0);
    m_preview->clearItems();
}

void LayoutSandboxWidget::removeItem(const QString& id)
{
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->id == id) {
            m_items.erase(it);
            break;
        }
    }

    // 重新构建表格
    m_itemTable->setRowCount(0);
    for (const auto& item : m_items) {
        addItemToTable(item);
    }

    updatePreview();
}

// ========== 从现有布局加载 ==========

void LayoutSandboxWidget::loadLayout(std::shared_ptr<BoxLayout> layout)
{
    if (!layout) return;

    clearItems();

    // 设置布局参数
    int left, top, right, bottom;
    layout->getContentsMargins(&left, &top, &right, &bottom);
    setMargins(left, top, right, bottom);
    setSpacing(layout->spacing());
    setLayoutDirection(layout->direction());

    // 加载布局项
    for (int i = 0; i < layout->count(); ++i) {
        auto item = layout->itemAt(i);
        if (!item) continue;

        SandboxItem sbItem;
        sbItem.id = QString("item_%1").arg(i);
        sbItem.sizeHint = (layout->direction() == BoxLayout::Direction::LeftToRight)
            ? item->sizeHint().width() : item->sizeHint().height();
        sbItem.minSize = (layout->direction() == BoxLayout::Direction::LeftToRight)
            ? item->minimumSize().width() : item->minimumSize().height();
        sbItem.maxSize = (layout->direction() == BoxLayout::Direction::LeftToRight)
            ? item->maximumSize().width() : item->maximumSize().height();
        sbItem.stretch = item->stretch();

        m_items.push_back(sbItem);
        addItemToTable(sbItem);
    }

    updatePreview();
}

// ========== 导出/导入 ==========

QString LayoutSandboxWidget::exportToJson() const
{
    QJsonObject root;

    root["containerWidth"] = m_widthSpin->value();
    root["containerHeight"] = m_heightSpin->value();
    root["direction"] = m_directionCombo->currentIndex();
    root["spacing"] = m_spacingSpin->value();
    root["leftMargin"] = m_leftMarginSpin->value();
    root["topMargin"] = m_topMarginSpin->value();
    root["rightMargin"] = m_rightMarginSpin->value();
    root["bottomMargin"] = m_bottomMarginSpin->value();

    QJsonArray itemsArray;
    for (const auto& item : m_items) {
        QJsonObject itemObj;
        itemObj["id"] = item.id;
        itemObj["sizeHint"] = item.sizeHint;
        itemObj["minSize"] = item.minSize;
        itemObj["maxSize"] = item.maxSize;
        itemObj["stretch"] = item.stretch;
        itemObj["isSpacing"] = item.isSpacing;
        itemsArray.append(itemObj);
    }
    root["items"] = itemsArray;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

void LayoutSandboxWidget::loadFromJson(const QString& json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("错误"), tr("JSON 解析失败: %1").arg(error.errorString()));
        return;
    }

    QJsonObject root = doc.object();

    clearItems();

    m_widthSpin->setValue(root["containerWidth"].toInt(400));
    m_heightSpin->setValue(root["containerHeight"].toInt(200));
    m_directionCombo->setCurrentIndex(root["direction"].toInt(0));
    m_spacingSpin->setValue(root["spacing"].toInt(8));
    m_leftMarginSpin->setValue(root["leftMargin"].toInt(8));
    m_topMarginSpin->setValue(root["topMargin"].toInt(8));
    m_rightMarginSpin->setValue(root["rightMargin"].toInt(8));
    m_bottomMarginSpin->setValue(root["bottomMargin"].toInt(8));

    QJsonArray itemsArray = root["items"].toArray();
    for (const auto& itemVal : itemsArray) {
        QJsonObject itemObj = itemVal.toObject();
        SandboxItem item;
        item.id = itemObj["id"].toString();
        item.sizeHint = itemObj["sizeHint"].toInt(100);
        item.minSize = itemObj["minSize"].toInt(0);
        item.maxSize = itemObj["maxSize"].toInt(1000000);
        item.stretch = itemObj["stretch"].toInt(0);
        item.isSpacing = itemObj["isSpacing"].toBool(false);

        m_items.push_back(item);
        addItemToTable(item);
    }

    updatePreview();
}

// ========== 槽函数 ==========

void LayoutSandboxWidget::onContainerSizeChanged()
{
    m_preview->setContainerSize(m_widthSpin->value(), m_heightSpin->value());
    m_preview->setDirection(static_cast<BoxLayout::Direction>(
        m_directionCombo->currentData().toInt()));
}

void LayoutSandboxWidget::onLayoutParamChanged()
{
    m_preview->setSpacing(m_spacingSpin->value());
    m_preview->setMargins(m_leftMarginSpin->value(), m_topMarginSpin->value(),
                          m_rightMarginSpin->value(), m_bottomMarginSpin->value());
}

void LayoutSandboxWidget::onAddItemClicked()
{
    QStringList types = {tr("固定尺寸"), tr("弹性项"), tr("间隔"), tr("自定义")};
    bool ok;
    QString type = QInputDialog::getItem(this, tr("添加布局项"),
                                          tr("选择类型:"), types, 0, false, &ok);
    if (!ok) return;

    SandboxItem item;

    if (type == tr("固定尺寸")) {
        QString id = QInputDialog::getText(this, tr("固定尺寸项"),
                                           tr("ID:"), QLineEdit::Normal, "item", &ok);
        if (!ok) return;

        int size = QInputDialog::getInt(this, tr("固定尺寸项"),
                                        tr("主方向尺寸 (px):"), 40, 1, 1000, 1, &ok);
        if (!ok) return;

        int crossSize = QInputDialog::getInt(this, tr("固定尺寸项"),
                                             tr("交叉方向尺寸 (px):\n(0 = 填满容器)"), 0, 0, 1000, 1, &ok);
        if (!ok) return;

        item.id = id;
        item.sizeHint = size;
        item.minSize = size;
        item.maxSize = size;
        // 交叉方向
        if (crossSize > 0) {
            item.crossSizeHint = crossSize;
            item.crossMinSize = crossSize;
            item.crossMaxSize = crossSize;
        }
        // else: 使用默认值 (0, 0, 1000000) = 填满容器
    }
    else if (type == tr("弹性项")) {
        QString id = QInputDialog::getText(this, tr("弹性项"),
                                           tr("ID:"), QLineEdit::Normal, "item", &ok);
        if (!ok) return;

        int stretch = QInputDialog::getInt(this, tr("弹性项"),
                                           tr("Stretch:"), 1, 1, 100, 1, &ok);
        if (!ok) return;

        item.id = id;
        item.sizeHint = 100;
        item.minSize = 0;
        item.maxSize = 1000000;
        item.stretch = stretch;
    }
    else if (type == tr("间隔")) {
        int size = QInputDialog::getInt(this, tr("间隔"),
                                        tr("尺寸 (px):"), 8, 1, 100, 1, &ok);
        if (!ok) return;

        item.id = QString("spacing_%1").arg(m_items.size());
        item.sizeHint = size;
        item.minSize = size;
        item.maxSize = size;
        item.isSpacing = true;
    }
    else { // 自定义
        QString id = QInputDialog::getText(this, tr("自定义项"),
                                           tr("ID:"), QLineEdit::Normal, "item", &ok);
        if (!ok) return;

        item.id = id;
        item.sizeHint = QInputDialog::getInt(this, tr("自定义项"),
                                             tr("SizeHint:"), 100, 0, 10000, 1, &ok);
        if (!ok) return;

        item.minSize = QInputDialog::getInt(this, tr("自定义项"),
                                            tr("MinSize:"), 0, 0, 10000, 1, &ok);
        if (!ok) return;

        item.maxSize = QInputDialog::getInt(this, tr("自定义项"),
                                            tr("MaxSize:"), 1000000, 1, 1000000, 1, &ok);
        if (!ok) return;

        item.stretch = QInputDialog::getInt(this, tr("自定义项"),
                                            tr("Stretch:"), 0, 0, 100, 1, &ok);
    }

    m_items.push_back(item);
    addItemToTable(item);
    updatePreview();
}

void LayoutSandboxWidget::onRemoveItemClicked()
{
    int row = m_itemTable->currentRow();
    if (row < 0 || row >= static_cast<int>(m_items.size())) return;

    m_items.erase(m_items.begin() + row);
    m_itemTable->removeRow(row);
    updatePreview();
}

void LayoutSandboxWidget::onItemTableCellChanged(int row, int column)
{
    if (row < 0 || row >= static_cast<int>(m_items.size())) return;

    auto* item = m_itemTable->item(row, column);
    if (!item) return;

    SandboxItem& sbItem = m_items[row];

    switch (column) {
    case 0: sbItem.id = item->text(); break;
    case 2: sbItem.sizeHint = item->text().toInt(); break;
    case 3: sbItem.stretch = item->text().toInt(); break;
    case 4: sbItem.minSize = item->text().toInt(); break;
    case 5: sbItem.maxSize = item->text().toInt(); break;
    case 6: // 交叉方向
        if (item->text() == "auto" || item->text() == "0") {
            sbItem.crossSizeHint = 0;
        } else {
            sbItem.crossSizeHint = item->text().toInt();
        }
        break;
    case 7: sbItem.crossMinSize = item->text().toInt(); break;
    case 8: sbItem.crossMaxSize = item->text().toInt(); break;
    }

    updatePreview();
}

void LayoutSandboxWidget::onPreviewItemClicked(int index)
{
    m_resultTable->selectRow(index);
    m_itemTable->selectRow(index);
}

void LayoutSandboxWidget::onLayoutComputed(const QString& summary)
{
    m_diagnosticLabel->setText(summary);
    updateResultTable();
}

// ========== 辅助方法 ==========

void LayoutSandboxWidget::updatePreview()
{
    m_preview->setItems(m_items);
}

void LayoutSandboxWidget::updateResultTable()
{
    const auto& items = m_preview->items();
    m_resultTable->setRowCount(static_cast<int>(items.size()));

    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];

        auto* idItem = new QTableWidgetItem(item.id);
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_resultTable->setItem(static_cast<int>(i), 0, idItem);

        auto setNumItem = [this, i](int col, int value) {
            auto* tableItem = new QTableWidgetItem(QString::number(value));
            tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
            m_resultTable->setItem(static_cast<int>(i), col, tableItem);
        };

        setNumItem(1, item.sizeHint);
        setNumItem(2, item.minSize);
        setNumItem(3, item.maxSize == 1000000 ? 9999 : item.maxSize);
        setNumItem(4, item.stretch);
        setNumItem(5, item.pos);
        setNumItem(6, item.size);
        setNumItem(7, item.crossSize);

        auto* statusItem = new QTableWidgetItem(itemStatus(item));
        statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
        if (item.isOverflow) {
            statusItem->setForeground(QColor(244, 67, 54));
        } else if (item.isCompressed) {
            statusItem->setForeground(QColor(255, 152, 0));
        }
        m_resultTable->setItem(static_cast<int>(i), 8, statusItem);
    }

    m_resultTable->resizeColumnsToContents();
}

void LayoutSandboxWidget::addItemToTable(const SandboxItem& item)
{
    int row = m_itemTable->rowCount();
    m_itemTable->insertRow(row);

    // 暂时断开信号，避免在设置时触发
    m_itemTable->blockSignals(true);

    // Type 列只读
    QString typeStr;
    if (item.isSpacing) {
        typeStr = tr("间隔");
    } else if (item.minSize == item.maxSize) {
        typeStr = tr("固定");
    } else if (item.stretch > 0) {
        typeStr = tr("弹性");
    } else {
        typeStr = tr("自定义");
    }

    m_itemTable->setItem(row, 0, new QTableWidgetItem(item.id));
    auto* typeItem = new QTableWidgetItem(typeStr);
    typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
    m_itemTable->setItem(row, 1, typeItem);
    m_itemTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.sizeHint)));
    m_itemTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.stretch)));
    m_itemTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.minSize)));
    m_itemTable->setItem(row, 5, new QTableWidgetItem(
        item.maxSize == 1000000 ? "∞" : QString::number(item.maxSize)));
    // 交叉方向
    m_itemTable->setItem(row, 6, new QTableWidgetItem(
        item.crossSizeHint == 0 ? "auto" : QString::number(item.crossSizeHint)));
    m_itemTable->setItem(row, 7, new QTableWidgetItem(QString::number(item.crossMinSize)));
    m_itemTable->setItem(row, 8, new QTableWidgetItem(
        item.crossMaxSize == 1000000 ? "∞" : QString::number(item.crossMaxSize)));

    m_itemTable->blockSignals(false);
    m_itemTable->resizeColumnsToContents();
}

QString LayoutSandboxWidget::itemStatus(const SandboxItem& item) const
{
    if (item.isSpacing) {
        return tr("间隔");
    }
    if (item.isOverflow) {
        return tr("✗溢出");
    }
    if (item.isCompressed) {
        return tr("⚠压缩");
    }
    if (item.minSize == item.maxSize) {
        return tr("✓固定");
    }
    if (item.stretch > 0) {
        return tr("✓拉伸");
    }
    return tr("正常");
}

} // namespace VLayout
