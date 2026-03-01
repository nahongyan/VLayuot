#include "vs_components.h"
#include "vs_theme.h"
#include "vs_utils.h"

#include <QFontMetrics>
#include <QApplication>

namespace VS {

// ============================================================================
// SolutionIconComponent
// ============================================================================

SolutionIconComponent::SolutionIconComponent(const QString& id)
    : VLayout::AbstractComponent(id)
{
    m_sizeHint = QSize(36, 36);
}

QSize SolutionIconComponent::sizeHint() const
{
    return m_sizeHint;
}

void SolutionIconComponent::paint(VLayout::ComponentContext& ctx)
{
    QString iconType = property("iconType").toString();
    QString name = property("name").toString();

    QColor bgColor;
    if (iconType == QStringLiteral("sln")) {
        bgColor = Theme::iconSln;
    } else if (iconType == QStringLiteral("folder")) {
        bgColor = Theme::iconFolder;
    } else {
        bgColor = Theme::iconCpp;
    }

    Utils::drawSolutionIcon(ctx.painter, geometry(), name, bgColor);
}

// ============================================================================
// ProjectInfoComponent
// ============================================================================

ProjectInfoComponent::ProjectInfoComponent(const QString& id)
    : VLayout::AbstractComponent(id)
{
    m_sizeHint = QSize(200, 42);
}

QSize ProjectInfoComponent::sizeHint() const
{
    return m_sizeHint;
}

void ProjectInfoComponent::paint(VLayout::ComponentContext& ctx)
{
    QString name = property("name").toString();
    QString path = property("path").toString();
    QRect r = geometry();

    ctx.painter->save();

    // 第一行：项目名称（粗体）
    QFont nameFont(QStringLiteral("Segoe UI"), 10);
    nameFont.setBold(true);
    ctx.painter->setFont(nameFont);
    ctx.painter->setPen(Theme::textPrimary);

    QRect nameRect(r.x(), r.y() + 2, r.width(), r.height() / 2);
    QString elidedName = ctx.painter->fontMetrics().elidedText(
        name, Qt::ElideMiddle, nameRect.width());
    ctx.painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignBottom, elidedName);

    // 第二行：路径（灰色小字）
    QFont pathFont(QStringLiteral("Segoe UI"), 8);
    ctx.painter->setFont(pathFont);
    ctx.painter->setPen(Theme::textSecond);

    QRect pathRect(r.x(), r.y() + r.height() / 2 + 1,
                   r.width(), r.height() / 2 - 2);
    QString elidedPath = ctx.painter->fontMetrics().elidedText(
        path, Qt::ElideMiddle, pathRect.width());
    ctx.painter->drawText(pathRect, Qt::AlignLeft | Qt::AlignTop, elidedPath);

    ctx.painter->restore();
}

// ============================================================================
// DateComponent
// ============================================================================

DateComponent::DateComponent(const QString& id)
    : VLayout::AbstractComponent(id)
{
    m_sizeHint = QSize(100, 20);
}

QSize DateComponent::sizeHint() const
{
    return m_sizeHint;
}

void DateComponent::paint(VLayout::ComponentContext& ctx)
{
    QDateTime dt = property("date").toDateTime();
    QString text = Utils::formatDate(dt);

    ctx.painter->save();
    QFont font(QStringLiteral("Segoe UI"), 9);
    ctx.painter->setFont(font);
    ctx.painter->setPen(Theme::textSecond);
    ctx.painter->drawText(geometry(), Qt::AlignRight | Qt::AlignVCenter, text);
    ctx.painter->restore();
}

// ============================================================================
// PinComponent
// ============================================================================

PinComponent::PinComponent(const QString& id)
    : VLayout::AbstractComponent(id)
{
    m_sizeHint = QSize(20, 20);
}

QSize PinComponent::sizeHint() const
{
    return m_sizeHint;
}

void PinComponent::paint(VLayout::ComponentContext& ctx)
{
    bool pinned = property("pinned").toBool();
    bool hovered = hasState(VLayout::ComponentState::Hovered);

    Utils::drawPinIcon(ctx.painter, geometry(), pinned, hovered);
}

} // namespace VS
