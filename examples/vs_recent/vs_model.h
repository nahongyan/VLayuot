#ifndef VS_MODEL_H
#define VS_MODEL_H

/**
 * @file vs_model.h
 * @brief 数据模型定义
 *
 * 定义了最近项目列表的数据模型，包括项目数据和分组逻辑。
 */

#include "vs_roles.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <vector>
#include <unordered_set>

namespace VS {

/**
 * @struct ProjectItem
 * @brief 项目数据结构
 *
 * 存储单个最近项目的所有数据。
 */
struct ProjectItem
{
    QString name;           ///< 项目名称
    QString path;           ///< 项目路径
    QDateTime lastAccess;   ///< 最后访问时间
    bool pinned = false;    ///< 是否已固定
    QString iconType;       ///< 图标类型: "sln" | "folder" | "cpp"
};

/**
 * @class RecentProjectsModel
 * @brief 最近项目数据模型
 *
 * 提供最近项目列表的数据，支持：
 * - 项目分组（已固定、本月、更早）
 * - 分组折叠/展开
 * - 固定状态切换
 * - 按时间和固定状态排序
 *
 * ## 分组逻辑
 * - 已固定：显示在最前面
 * - 本月：最近30天内访问的项目
 * - 更早：超过30天的项目
 */
class RecentProjectsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RecentProjectsModel(QObject* parent = nullptr);

    // ========== QAbstractListModel 接口 ==========

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // ========== 数据管理 ==========

    /**
     * @brief 加载示例数据
     */
    void loadSampleData();

    /**
     * @brief 添加项目
     */
    void addProject(const ProjectItem& item);

    /**
     * @brief 清空所有项目
     */
    void clear();

    /**
     * @brief 刷新分组
     */
    void refreshGroups();

    // ========== 折叠/展开 ==========

    /**
     * @brief 切换分组折叠状态
     */
    void toggleGroupCollapsed(int groupIndex);

    /**
     * @brief 检查分组是否折叠
     */
    bool isGroupCollapsed(int groupIndex) const;

private:
    /**
     * @enum ItemType
     * @brief 行类型
     */
    enum class ItemType {
        Group,      ///< 分组标题
        Project     ///< 项目行
    };

    /**
     * @struct Row
     * @brief 模型行数据
     */
    struct Row {
        ItemType type;          ///< 行类型
        int groupIndex = -1;    ///< 分组索引
        int projectIndex = -1;  ///< 项目索引（仅对项目行有效）
        QString groupTitle;     ///< 分组标题（仅对分组标题有效）
        int projectCount = 0;   ///< 分组内项目数
    };

    void buildVisibleRows();
    QString buildGroupTitle(int groupIndex, int count) const;

    // ========== 成员变量 ==========

    std::vector<ProjectItem> m_items;           ///< 原始项目数据
    std::vector<Row> m_allRows;                 ///< 所有行（包括折叠的）
    std::vector<Row> m_visibleRows;             ///< 可见行（过滤折叠后）
    std::unordered_set<int> m_collapsedGroups;  ///< 折叠的分组索引集合
};

} // namespace VS

#endif // VS_MODEL_H
