#ifndef VS_ROLES_H
#define VS_ROLES_H

/**
 * @file vs_roles.h
 * @brief 自定义数据角色定义
 *
 * 定义了最近项目列表视图使用的自定义 Qt ItemDataRole。
 */

#include <Qt>

namespace VS {

/**
 * @enum Roles
 * @brief 自定义数据角色
 *
 * 用于在 Model 中存储和检索项目数据。
 */
enum Roles {
    NameRole       = Qt::UserRole + 1,  ///< 项目名称
    PathRole,                           ///< 项目路径
    DateRole,                           ///< 最后访问时间
    PinnedRole,                         ///< 是否已固定
    IconTypeRole,                       ///< 图标类型 ("sln" | "folder" | "cpp")
    IsGroupRole,                        ///< 是否为分组标题行
    GroupIndexRole,                     ///< 分组索引
    CollapsedRole,                      ///< 分组是否折叠
    ProjectCountRole,                   ///< 分组内项目数量
};

} // namespace VS

#endif // VS_ROLES_H
