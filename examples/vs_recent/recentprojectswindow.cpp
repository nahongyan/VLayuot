#include "recentprojectswindow.h"
#include "vs_components.h"
#include "vs_model.h"
#include "vs_delegates.h"
#include "vs_theme.h"

#include <QListView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QApplication>

namespace VS {

// ============================================================================
// RecentProjectsWindow
// ============================================================================

RecentProjectsWindow::RecentProjectsWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    setupStyleSheet();
    setupData();
}

RecentProjectsWindow::~RecentProjectsWindow()
{
}

void RecentProjectsWindow::setupUI()
{
    // 设置窗口属性
    setWindowTitle(tr("Recent Projects"));
    resize(1100, 800);

    // 创建中央部件
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 标题栏 ==========

    QFrame* headerFrame = new QFrame(this);
    headerFrame->setObjectName(QStringLiteral("headerFrame"));
    headerFrame->setFixedHeight(48);

    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(12);

    // 标题标签
    QLabel* titleLabel = new QLabel(tr("Recent"), headerFrame);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    // 搜索框（占位，暂不实现功能）
    QLineEdit* searchEdit = new QLineEdit(headerFrame);
    searchEdit->setObjectName(QStringLiteral("searchEdit"));
    searchEdit->setPlaceholderText(tr("Search projects..."));
    searchEdit->setFixedWidth(200);
    headerLayout->addWidget(searchEdit);

    mainLayout->addWidget(headerFrame);

    // ========== 分隔线 ==========

    QFrame* separator = new QFrame(this);
    separator->setObjectName(QStringLiteral("separator"));
    separator->setFixedHeight(1);
    mainLayout->addWidget(separator);

    // ========== 列表视图 ==========

    m_listView = new QListView(this);
    m_listView->setObjectName(QStringLiteral("projectList"));

    // 基本设置
    m_listView->setFrameShape(QFrame::NoFrame);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 启用鼠标追踪以支持悬停效果
    m_listView->setMouseTracking(true);
    m_listView->viewport()->setAttribute(Qt::WA_Hover);

    mainLayout->addWidget(m_listView);

    // ========== 委托 ==========

    m_delegate = new RecentProjectsDelegate(this);
    m_listView->setItemDelegate(m_delegate);

    // 连接悬停信号，用于改变光标
    connect(m_delegate, &RecentProjectsDelegate::clickableHoverChanged,
            this, [this](const QString& componentId, bool hovered) {
                Q_UNUSED(componentId);
                if (hovered) {
                    m_listView->viewport()->setCursor(Qt::PointingHandCursor);
                } else {
                    m_listView->viewport()->unsetCursor();
                }
            });
}

void RecentProjectsWindow::setupStyleSheet()
{
    QString style = QStringLiteral(R"(
        /* 主窗口 */
        QMainWindow {
            background-color: %1;
        }

        /* 标题栏 */
        #headerFrame {
            background-color: %2;
        }

        #titleLabel {
            color: %3;
            font-size: 14pt;
            font-weight: bold;
        }

        /* 分隔线 */
        #separator {
            background-color: %4;
        }

        /* 搜索框 */
        #searchEdit {
            background-color: %1;
            border: 1px solid %4;
            border-radius: 3px;
            padding: 4px 8px;
            color: %3;
            selection-background-color: %5;
        }

        #searchEdit:focus {
            border-color: %6;
        }

        #searchEdit::placeholder {
            color: %7;
        }

        /* 列表视图 */
        #projectList {
            background-color: %2;
            outline: none;
            border: none;
        }

        #projectList::item {
            border: none;
            outline: none;
            padding: 0;
            background-color: transparent;
        }

        #projectList::item:selected {
            background-color: %5;
        }

        #projectList::item:hover:!selected {
            background-color: %10;
        }

        /* 滚动条 */
        QScrollBar:vertical {
            background-color: %8;
            width: 10px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background-color: %9;
            min-height: 30px;
            border-radius: 5px;
            margin: 2px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %3;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )").arg(
        Theme::bgBase.name(),          // %1
        Theme::bgSurface.name(),       // %2
        Theme::textPrimary.name(),     // %3
        Theme::separator.name(),       // %4
        Theme::bgSelected.name(),      // %5
        Theme::accentBlue.name(),      // %6
        Theme::textSecond.name(),      // %7
        Theme::scrollBg.name(),        // %8
        Theme::scrollHandle.name(),    // %9
        Theme::bgHover.name()          // %10
    );

    setStyleSheet(style);
}

void RecentProjectsWindow::setupData()
{
    m_model = new RecentProjectsModel(this);
    m_model->loadSampleData();

    m_listView->setModel(m_model);
}

} // namespace VS
