#ifndef VS_COMPONENTS_H
#define VS_COMPONENTS_H

/**
 * @file vs_components.h
 * @brief 自定义组件定义
 *
 * 定义了用于最近项目列表的自定义绘制组件。
 */

#include "vlayout/framework.h"

namespace VS {

/**
 * @class SolutionIconComponent
 * @brief 解决方案图标组件
 *
 * 绘制项目图标，根据类型显示不同颜色和符号。
 */
class SolutionIconComponent : public VLayout::AbstractComponent
{
public:
    explicit SolutionIconComponent(const QString& id);

    QString type() const override { return QStringLiteral("SolutionIcon"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;
};

/**
 * @class ProjectInfoComponent
 * @brief 项目信息组件
 *
 * 绘制两行文本：项目名称（粗体）+ 项目路径（灰色）。
 */
class ProjectInfoComponent : public VLayout::AbstractComponent
{
public:
    explicit ProjectInfoComponent(const QString& id);

    QString type() const override { return QStringLiteral("ProjectInfo"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;
};

/**
 * @class DateComponent
 * @brief 日期组件
 *
 * 显示格式化后的日期时间。
 */
class DateComponent : public VLayout::AbstractComponent
{
public:
    explicit DateComponent(const QString& id);

    QString type() const override { return QStringLiteral("Date"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;
};

/**
 * @class PinComponent
 * @brief 图钉组件
 *
 * 绘制固定/取消固定按钮。
 */
class PinComponent : public VLayout::AbstractComponent
{
public:
    explicit PinComponent(const QString& id);

    QString type() const override { return QStringLiteral("Pin"); }
    QSize sizeHint() const override;
    void paint(VLayout::ComponentContext& ctx) override;
};

} // namespace VS

#endif // VS_COMPONENTS_H
