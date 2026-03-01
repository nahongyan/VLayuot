#ifndef RECENTPROJECTSWINDOW_H
#define RECENTPROJECTSWINDOW_H

/**
 * @file recentprojectswindow.h
 * @brief 主窗口定义
 *
 * 定义了最近项目列表的主窗口，使用 QListView 显示分组项目。
 */

#include <QMainWindow>

class QListView;

namespace VS {

class RecentProjectsModel;
class RecentProjectsDelegate;

/**
 * @class RecentProjectsWindow
 * @brief 最近项目主窗口
 *
 * 显示 Visual Studio 风格的最近项目列表。
 *
 * ## 布局
 * - 顶部：标题栏（"最近" + 搜索框）
 * - 中间：项目列表视图
 *
 * ## 功能
 * - 显示最近打开的项目
 * - 分组：已固定 / 本月 / 更早
 * - 支持折叠/展开分组
 * - 支持固定/取消固定项目
 */
class RecentProjectsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RecentProjectsWindow(QWidget* parent = nullptr);
    ~RecentProjectsWindow() override;

private:
    void setupUI();
    void setupStyleSheet();
    void setupData();

private:
    QListView* m_listView = nullptr;                    ///< 列表视图
    RecentProjectsModel* m_model = nullptr;             ///< 数据模型
    RecentProjectsDelegate* m_delegate = nullptr;       ///< 组合委托
};

} // namespace VS

#endif // RECENTPROJECTSWINDOW_H
