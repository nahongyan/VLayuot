#include "vs_model.h"

#include <algorithm>

namespace VS {

// ============================================================================
// RecentProjectsModel
// ============================================================================

RecentProjectsModel::RecentProjectsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int RecentProjectsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_visibleRows.size());
}

QVariant RecentProjectsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_visibleRows.size())) {
        return QVariant();
    }

    const Row& row = m_visibleRows[index.row()];

    // 分组标题行
    if (row.type == ItemType::Group) {
        switch (role) {
        case Qt::DisplayRole:
            return row.groupTitle;
        case VS::IsGroupRole:
            return true;
        case VS::GroupIndexRole:
            return row.groupIndex;
        case VS::CollapsedRole:
            return m_collapsedGroups.count(row.groupIndex) > 0;
        case VS::ProjectCountRole:
            return row.projectCount;
        default:
            return QVariant();
        }
    }

    // 项目行
    if (row.projectIndex < 0 || row.projectIndex >= static_cast<int>(m_items.size())) {
        return QVariant();
    }

    const ProjectItem& item = m_items[row.projectIndex];

    switch (role) {
    case Qt::DisplayRole:
        return item.name;

    case VS::NameRole:
        return item.name;

    case VS::PathRole:
        return item.path;

    case VS::DateRole:
        return item.lastAccess;

    case VS::PinnedRole:
        return item.pinned;

    case VS::IconTypeRole:
        return item.iconType;

    case VS::IsGroupRole:
        return false;

    case VS::GroupIndexRole:
        return row.groupIndex;

    default:
        return QVariant();
    }
}

bool RecentProjectsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (index.row() >= static_cast<int>(m_visibleRows.size())) {
        return false;
    }

    const Row& row = m_visibleRows[index.row()];

    // 分组行：处理折叠状态
    if (row.type == ItemType::Group) {
        if (role == VS::CollapsedRole) {
            toggleGroupCollapsed(row.groupIndex);
            return true;
        }
        return false;
    }

    // 项目行：只允许修改 PinnedRole
    if (role != VS::PinnedRole) {
        return false;
    }

    if (row.projectIndex < 0 || row.projectIndex >= static_cast<int>(m_items.size())) {
        return false;
    }

    ProjectItem& item = m_items[row.projectIndex];
    bool newPinned = value.toBool();

    if (item.pinned != newPinned) {
        item.pinned = newPinned;

        // 发出数据变更信号
        emit dataChanged(index, index, {VS::PinnedRole});

        // 重新排序和分组
        beginResetModel();
        refreshGroups();
        endResetModel();

        return true;
    }

    return false;
}

Qt::ItemFlags RecentProjectsModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);

    if (index.isValid()) {
        if (index.row() < static_cast<int>(m_visibleRows.size())) {
            const Row& row = m_visibleRows[index.row()];
            if (row.type == ItemType::Group) {
                // 分组标题不可选中，但可点击
                f &= ~Qt::ItemIsSelectable;
            }
        }
    }

    return f;
}

void RecentProjectsModel::loadSampleData()
{
    beginResetModel();

    m_items.clear();
    m_collapsedGroups.clear();

    // 添加示例项目
    ProjectItem item;

    item.name = QStringLiteral("MyApplication.sln");
    item.path = QStringLiteral("C:\\Projects\\MyApplication\\MyApplication.sln");
    item.lastAccess = QDateTime::currentDateTime().addSecs(-3600);  // 1小时前
    item.pinned = true;
    item.iconType = QStringLiteral("sln");
    m_items.push_back(item);

    item.name = QStringLiteral("QtWidgetsApp");
    item.path = QStringLiteral("D:\\Dev\\QtWidgetsApp\\QtWidgetsApp.pro");
    item.lastAccess = QDateTime::currentDateTime().addDays(-2);  // 2天前
    item.pinned = false;
    item.iconType = QStringLiteral("folder");
    m_items.push_back(item);

    item.name = QStringLiteral("NetworkLibrary");
    item.path = QStringLiteral("E:\\Libraries\\NetworkLibrary\\NetworkLibrary.vcxproj");
    item.lastAccess = QDateTime::currentDateTime().addDays(-10);  // 10天前
    item.pinned = true;
    item.iconType = QStringLiteral("cpp");
    m_items.push_back(item);

    item.name = QStringLiteral("DataParser");
    item.path = QStringLiteral("C:\\Work\\DataParser\\DataParser.cpp");
    item.lastAccess = QDateTime::currentDateTime().addDays(-45);  // 45天前
    item.pinned = false;
    item.iconType = QStringLiteral("cpp");
    m_items.push_back(item);

    item.name = QStringLiteral("OldProject.sln");
    item.path = QStringLiteral("F:\\Archive\\OldProject\\OldProject.sln");
    item.lastAccess = QDateTime::currentDateTime().addDays(-90);  // 90天前
    item.pinned = false;
    item.iconType = QStringLiteral("sln");
    m_items.push_back(item);

    // 刷新分组
    refreshGroups();

    endResetModel();
}

void RecentProjectsModel::addProject(const ProjectItem& item)
{
    beginInsertRows(QModelIndex(), static_cast<int>(m_items.size()),
                    static_cast<int>(m_items.size()));
    m_items.push_back(item);
    refreshGroups();
    endInsertRows();
}

void RecentProjectsModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_allRows.clear();
    m_visibleRows.clear();
    m_collapsedGroups.clear();
    endResetModel();
}

void RecentProjectsModel::refreshGroups()
{
    m_allRows.clear();

    // 按固定状态和时间排序
    std::sort(m_items.begin(), m_items.end(), [](const ProjectItem& a, const ProjectItem& b) {
        // 固定的项目排在前面
        if (a.pinned != b.pinned) {
            return a.pinned > b.pinned;
        }
        // 然后按时间降序
        return a.lastAccess > b.lastAccess;
    });

    QDateTime now = QDateTime::currentDateTime();
    int currentGroup = -1;  // -1=未分组, 0=已固定, 1=本月, 2=更早
    int groupStartIndex = -1;

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        const ProjectItem& item = m_items[i];
        int group = -1;

        if (item.pinned) {
            group = 0;  // 已固定
        } else {
            int daysAgo = item.lastAccess.daysTo(now);
            if (daysAgo <= 30) {
                group = 1;  // 本月
            } else {
                group = 2;  // 更早
            }
        }

        // 新分组开始
        if (group != currentGroup) {
            // 更新上一个分组的项目数
            if (currentGroup >= 0 && groupStartIndex >= 0) {
                m_allRows[groupStartIndex].projectCount =
                    static_cast<int>(m_allRows.size()) - groupStartIndex - 1;
            }

            // 添加分组标题
            Row groupRow;
            groupRow.type = ItemType::Group;
            groupRow.groupIndex = group;
            groupRow.groupTitle = buildGroupTitle(group, 0);  // 先设为0，后面更新
            groupStartIndex = static_cast<int>(m_allRows.size());
            m_allRows.push_back(groupRow);
            currentGroup = group;
        }

        // 添加项目行
        Row projectRow;
        projectRow.type = ItemType::Project;
        projectRow.groupIndex = group;
        projectRow.projectIndex = i;
        m_allRows.push_back(projectRow);
    }

    // 更新最后一个分组的项目数
    if (currentGroup >= 0 && groupStartIndex >= 0) {
        m_allRows[groupStartIndex].projectCount =
            static_cast<int>(m_allRows.size()) - groupStartIndex - 1;
        // 更新标题中的数量
        m_allRows[groupStartIndex].groupTitle =
            buildGroupTitle(currentGroup, m_allRows[groupStartIndex].projectCount);
    }

    // 构建可见行
    buildVisibleRows();
}

void RecentProjectsModel::buildVisibleRows()
{
    m_visibleRows.clear();

    for (const Row& row : m_allRows) {
        if (row.type == ItemType::Group) {
            // 分组标题总是显示
            m_visibleRows.push_back(row);
        } else {
            // 项目行：检查所在分组是否折叠
            if (m_collapsedGroups.count(row.groupIndex) == 0) {
                m_visibleRows.push_back(row);
            }
        }
    }
}

void RecentProjectsModel::toggleGroupCollapsed(int groupIndex)
{
    if (m_collapsedGroups.count(groupIndex) > 0) {
        m_collapsedGroups.erase(groupIndex);
    } else {
        m_collapsedGroups.insert(groupIndex);
    }

    beginResetModel();
    buildVisibleRows();
    endResetModel();
}

bool RecentProjectsModel::isGroupCollapsed(int groupIndex) const
{
    return m_collapsedGroups.count(groupIndex) > 0;
}

QString RecentProjectsModel::buildGroupTitle(int groupIndex, int count) const
{
    QString baseTitle;
    switch (groupIndex) {
    case 0:
        baseTitle = tr("Pinned");
        break;
    case 1:
        baseTitle = tr("This Month");
        break;
    case 2:
        baseTitle = tr("Earlier");
        break;
    default:
        baseTitle = tr("Projects");
        break;
    }

    if (count > 0) {
        return baseTitle + QStringLiteral(" (%1)").arg(count);
    }
    return baseTitle;
}

} // namespace VS
