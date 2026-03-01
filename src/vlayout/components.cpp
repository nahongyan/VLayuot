#include "vlayout/components.h"
#include <QFontMetrics>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPainterPath>
#include <QIcon>

namespace VLayout {

// ============================================================================
// 默认颜色定义
// ============================================================================

namespace {
    const QColor DEFAULT_BG(45, 50, 65);
    const QColor DEFAULT_BG_HOVER(55, 60, 75);
    const QColor DEFAULT_BG_DISABLED(60, 60, 60);
    const QColor DEFAULT_TEXT(204, 204, 204);
    const QColor DEFAULT_TEXT_DISABLED(128, 128, 128);
    const QColor DEFAULT_ACCENT(0, 122, 204);
    const QColor DEFAULT_ACCENT_HOVER(0, 100, 180);
    const QColor DEFAULT_BORDER(80, 85, 100);
    const QColor DEFAULT_BORDER_HOVER(100, 105, 120);
}

// ============================================================================
// QStyle 辅助函数
// ============================================================================

/**
 * @brief 从 ComponentContext 初始化 QStyleOption
 *
 * 将组件状态转换为 Qt 样式系统的状态标志，
 * 以便使用 QStyle 进行标准控件绘制。
 */
void initFromContext(QStyleOption& opt, const ComponentContext& ctx,
                     const QRect& rect, int compStates)
{
    opt.rect = rect;

    if (ctx.option) {
        opt.direction = ctx.option->direction;
        opt.fontMetrics = ctx.option->fontMetrics;
        opt.palette = ctx.option->palette;
        opt.styleObject = ctx.option->styleObject;
    } else {
        opt.palette = QApplication::palette();
        opt.fontMetrics = QFontMetrics(ctx.font);
    }

    // 转换状态标志
    QStyle::State ss = QStyle::State_None;
    if (compStates & int(ComponentState::Enabled)) {
        ss |= QStyle::State_Enabled;
    }
    if (ctx.isHovered() || (compStates & int(ComponentState::Hovered))) {
        ss |= QStyle::State_MouseOver;
    }
    if (compStates & int(ComponentState::Pressed)) {
        ss |= QStyle::State_Sunken;
    } else {
        ss |= QStyle::State_Raised;
    }
    if (compStates & int(ComponentState::Focused)) {
        ss |= QStyle::State_HasFocus;
    }
    if (compStates & int(ComponentState::Checked)) {
        ss |= QStyle::State_On;
    } else {
        ss |= QStyle::State_Off;
    }

    opt.state = ss;
}

// ============================================================================
// LabelComponent 实现
// ============================================================================

LabelComponent::LabelComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 20);
}

QSize LabelComponent::sizeHint() const
{
    QString t = text();
    if (!t.isEmpty()) {
        QFont defaultFont;
        QFontMetrics fm(defaultFont);
        return QSize(fm.horizontalAdvance(t), fm.height());
    }
    return m_sizeHint;
}

void LabelComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();

    // 设置字体
    QFont f = ctx.font;
    if (hasProperty("font")) {
        f = property("font").value<QFont>();
    }
    p->setFont(f);

    // 设置文本颜色
    QColor textColor = ctx.textColor;
    if (hasProperty("color")) {
        textColor = property("color").value<QColor>();
    }
    p->setPen(textColor);

    // 获取文本并处理省略
    QString text = property("text").toString();
    QString displayText = text;

    if (!m_wordWrap) {
        QFontMetrics fm(f);
        displayText = fm.elidedText(text, m_elideMode, r.width());
    }

    // 绘制文本
    if (m_wordWrap) {
        p->drawText(r, m_alignment | Qt::TextWordWrap, displayText);
    } else {
        p->drawText(r, m_alignment, displayText);
    }
}

// ============================================================================
// ButtonComponent 实现
// ============================================================================

ButtonComponent::ButtonComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(80, 28);
}

QSize ButtonComponent::sizeHint() const
{
    QStyle* style = QApplication::style();
    QStyleOptionButton opt;
    opt.text = m_text;

    QFontMetrics fm(QApplication::font());
    QSize content(fm.horizontalAdvance(m_text) + 12, fm.height());
    return style->sizeFromContents(QStyle::CT_PushButton, &opt, content);
}

void ButtonComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionButton opt;
    initFromContext(opt, ctx, r, m_states);
    opt.text = m_text;

    if (m_flat) {
        opt.features |= QStyleOptionButton::Flat;
    }

    style->drawControl(QStyle::CE_PushButton, &opt, p);
}

void ButtonComponent::onClick(const QPoint& pos)
{
    Q_UNUSED(pos);
    if (m_checkable) {
        setState(ComponentState::Checked, !isChecked());
    }
}

// ============================================================================
// CheckBoxComponent 实现
// ============================================================================

CheckBoxComponent::CheckBoxComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 22);
}

QSize CheckBoxComponent::sizeHint() const
{
    QStyle* style = QApplication::style();
    QStyleOptionButton opt;
    opt.text = m_text;

    QFontMetrics fm(QApplication::font());
    QSize content(fm.horizontalAdvance(m_text), fm.height());
    return style->sizeFromContents(QStyle::CT_CheckBox, &opt, content);
}

void CheckBoxComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionButton opt;
    initFromContext(opt, ctx, r, m_states);
    opt.text = m_text;

    style->drawControl(QStyle::CE_CheckBox, &opt, p);
}

void CheckBoxComponent::onClick(const QPoint& pos)
{
    Q_UNUSED(pos);
    setState(ComponentState::Checked, !isChecked());
}

// ============================================================================
// ProgressBarComponent 实现
// ============================================================================

ProgressBarComponent::ProgressBarComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 4);
}

QSize ProgressBarComponent::sizeHint() const
{
    QStyleOptionProgressBar opt;
    opt.minimum = m_min;
    opt.maximum = m_max;
    return QApplication::style()->sizeFromContents(
        QStyle::CT_ProgressBar, &opt, QSize(100, 20));
}

void ProgressBarComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionProgressBar opt;
    initFromContext(opt, ctx, r, m_states);
    opt.minimum = m_min;
    opt.maximum = m_max;
    opt.progress = m_value;
    opt.textVisible = m_textVisible;
    opt.textAlignment = Qt::AlignCenter;

    if (m_textVisible && m_max > m_min) {
        opt.text = QString("%1%").arg(
            int(100.0 * (m_value - m_min) / (m_max - m_min)));
    }

    style->drawControl(QStyle::CE_ProgressBar, &opt, p);
}

// ============================================================================
// IconComponent 实现
// ============================================================================

IconComponent::IconComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(24, 24);
}

QSize IconComponent::sizeHint() const
{
    return m_iconSize.isEmpty() ? m_sizeHint : m_iconSize;
}

void IconComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();

    // 绘制背景色块
    if (m_color.isValid()) {
        p->setBrush(m_color);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(r, 4, 4);
    }

    // 绘制图标字符
    if (!m_icon.isEmpty()) {
        QFont iconFont = p->font();
        iconFont.setPointSize(r.height() * 2 / 3);
        p->setFont(iconFont);
        p->setPen(m_color.isValid() ? QColor(255, 255, 255) : ctx.textColor);
        p->drawText(r, Qt::AlignCenter, m_icon);
    }
}

// ============================================================================
// BadgeComponent 实现
// ============================================================================

BadgeComponent::BadgeComponent(const QString& id)
    : AbstractComponent(id)
    , m_color(220, 53, 69)
{
    m_sizeHint = QSize(20, 18);
}

QSize BadgeComponent::sizeHint() const
{
    if (!m_text.isEmpty()) {
        QFont defaultFont;
        QFontMetrics fm(defaultFont);
        int w = fm.horizontalAdvance(m_text) + 10;
        return QSize(qMax(w, 18), 18);
    }
    return m_sizeHint;
}

void BadgeComponent::paint(ComponentContext& ctx)
{
    if (!isVisible()) return;

    QPainter* p = ctx.painter;
    QRect r = geometry();

    QColor color = hasProperty("color") ? property("color").value<QColor>() : m_color;
    QString text = hasProperty("text") ? property("text").toString() : m_text;

    if (text.isEmpty()) return;

    // 绘制圆角背景
    p->setBrush(color);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(r, 9, 9);

    // 绘制文本
    QFont f = p->font();
    f.setPointSize(8);
    f.setBold(true);
    p->setFont(f);
    p->setPen(QColor(255, 255, 255));
    p->drawText(r, Qt::AlignCenter, text);
}

// ============================================================================
// SeparatorComponent 实现
// ============================================================================

SeparatorComponent::SeparatorComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(1, 1);
}

QSize SeparatorComponent::sizeHint() const
{
    if (m_orientation == Qt::Horizontal) {
        return QSize(100, 1);
    }
    return QSize(1, 100);
}

void SeparatorComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();

    p->setPen(QPen(DEFAULT_BORDER, 1));

    if (m_orientation == Qt::Horizontal) {
        int y = r.center().y();
        p->drawLine(r.left(), y, r.right(), y);
    } else {
        int x = r.center().x();
        p->drawLine(x, r.top(), x, r.bottom());
    }
}

// ============================================================================
// AvatarComponent 实现
// ============================================================================

AvatarComponent::AvatarComponent(const QString& id)
    : AbstractComponent(id)
    , m_size(40)
{
    m_sizeHint = QSize(m_size, m_size);
}

QSize AvatarComponent::sizeHint() const
{
    return QSize(m_size, m_size);
}

void AvatarComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();

    if (!m_pixmap.isNull()) {
        // 绘制图片头像
        p->drawPixmap(r, m_pixmap.scaled(r.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // 绘制文字头像
        QColor bgColor = ctx.accentColor;
        if (hasProperty("color")) {
            bgColor = property("color").value<QColor>();
        }

        // 绘制圆形背景
        p->setBrush(bgColor);
        p->setPen(Qt::NoPen);
        p->drawEllipse(r);

        // 绘制首字母
        if (!m_text.isEmpty()) {
            QString initial = m_text.left(1).toUpper();
            QFont f = p->font();
            f.setPointSize(r.height() / 2);
            f.setBold(true);
            p->setFont(f);
            p->setPen(QColor(255, 255, 255));
            p->drawText(r, Qt::AlignCenter, initial);
        }
    }
}

// ============================================================================
// SwitchComponent 实现
// ============================================================================

SwitchComponent::SwitchComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(44, 24);
}

QSize SwitchComponent::sizeHint() const
{
    return m_sizeHint;
}

void SwitchComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    p->setRenderHint(QPainter::Antialiasing);

    bool on = isChecked();

    // 绘制轨道
    QColor trackColor = on ? DEFAULT_ACCENT : QColor(80, 90, 110);
    p->setBrush(trackColor);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(r, 12, 12);

    // 绘制滑块
    int thumbSize = 20;
    int thumbY = r.top() + (r.height() - thumbSize) / 2;
    int thumbX = on ? r.right() - thumbSize - 2 : r.left() + 2;

    QRect thumbRect(thumbX, thumbY, thumbSize, thumbSize);
    p->setBrush(QColor(255, 255, 255));
    p->drawEllipse(thumbRect);
}

void SwitchComponent::onClick(const QPoint&)
{
    setState(ComponentState::Checked, !isChecked());
}

// ============================================================================
// SliderComponent 实现
// ============================================================================

SliderComponent::SliderComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 20);
}

QSize SliderComponent::sizeHint() const
{
    QStyleOptionSlider opt;
    opt.orientation = m_orientation;
    opt.minimum = m_min;
    opt.maximum = m_max;
    return QApplication::style()->sizeFromContents(
        QStyle::CT_Slider, &opt, QSize(100, 20));
}

void SliderComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionSlider opt;
    initFromContext(opt, ctx, r, m_states);
    opt.minimum = m_min;
    opt.maximum = m_max;
    opt.sliderPosition = m_value;
    opt.sliderValue = m_value;
    opt.orientation = m_orientation;
    opt.singleStep = 1;
    opt.pageStep = qMax(1, (m_max - m_min) / 10);
    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;

    if (ctx.isHovered()) {
        opt.activeSubControls = QStyle::SC_SliderHandle;
    }

    style->drawComplexControl(QStyle::CC_Slider, &opt, p);
}

// ============================================================================
// ComboBoxComponent 实现
// ============================================================================

ComboBoxComponent::ComboBoxComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(120, 28);
}

QSize ComboBoxComponent::sizeHint() const
{
    QStyle* style = QApplication::style();
    QStyleOptionComboBox opt;
    QFontMetrics fm(QApplication::font());

    int maxW = 0;
    for (const QString& item : m_items) {
        maxW = qMax(maxW, fm.horizontalAdvance(item));
    }
    QSize content(maxW, fm.height());
    return style->sizeFromContents(QStyle::CT_ComboBox, &opt, content);
}

void ComboBoxComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionComboBox opt;
    initFromContext(opt, ctx, r, m_states);
    opt.currentText = currentText();
    opt.editable = false;
    opt.subControls = QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxArrow
                    | QStyle::SC_ComboBoxEditField;

    style->drawComplexControl(QStyle::CC_ComboBox, &opt, p);

    // 绘制当前文本
    QRect textRect = style->subControlRect(
        QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField);
    p->setPen(opt.palette.color(
        opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled,
        QPalette::Text));
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.currentText);
}

void ComboBoxComponent::onClick(const QPoint&)
{
    if (!m_items.isEmpty()) {
        m_currentIndex = (m_currentIndex + 1) % m_items.size();
    }
}

void ComboBoxComponent::setCurrentIndex(int index)
{
    if (index >= -1 && index < m_items.size()) {
        m_currentIndex = index;
    }
}

QString ComboBoxComponent::currentText() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        return m_items[m_currentIndex];
    }
    return QString();
}

// ============================================================================
// SpinBoxComponent 实现
// ============================================================================

SpinBoxComponent::SpinBoxComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(80, 28);
}

QSize SpinBoxComponent::sizeHint() const
{
    QStyleOptionSpinBox opt;
    opt.frame = true;
    QFontMetrics fm(QApplication::font());
    QString sample = QString::number(m_max) + m_suffix;
    QSize content(fm.horizontalAdvance(sample) + 4, fm.height());
    return QApplication::style()->sizeFromContents(
        QStyle::CT_SpinBox, &opt, content);
}

void SpinBoxComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    QStyle* style = QApplication::style();

    QStyleOptionSpinBox opt;
    initFromContext(opt, ctx, r, m_states);
    opt.frame = true;
    opt.buttonSymbols = QAbstractSpinBox::UpDownArrows;
    opt.subControls = QStyle::SC_SpinBoxFrame | QStyle::SC_SpinBoxUp
                    | QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxEditField;
    opt.stepEnabled = QAbstractSpinBox::StepNone;

    if (m_value > m_min) opt.stepEnabled |= QAbstractSpinBox::StepDownEnabled;
    if (m_value < m_max) opt.stepEnabled |= QAbstractSpinBox::StepUpEnabled;

    style->drawComplexControl(QStyle::CC_SpinBox, &opt, p);

    // 绘制数值文本
    QRect textRect = style->subControlRect(
        QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxEditField);
    QString text = QString::number(m_value) + m_suffix;
    p->setPen(opt.palette.color(QPalette::Text));
    p->drawText(textRect, Qt::AlignCenter, text);
}

void SpinBoxComponent::onClick(const QPoint& pos)
{
    QStyle* style = QApplication::style();
    QStyleOptionSpinBox opt;
    opt.rect = geometry();
    opt.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;
    opt.frame = true;
    opt.buttonSymbols = QAbstractSpinBox::UpDownArrows;

    QStyle::SubControl sc = style->hitTestComplexControl(
        QStyle::CC_SpinBox, &opt, pos);

    if (sc == QStyle::SC_SpinBoxUp) {
        m_value = qMin(m_max, m_value + m_step);
    } else if (sc == QStyle::SC_SpinBoxDown) {
        m_value = qMax(m_min, m_value - m_step);
    }
}

// ============================================================================
// RatingComponent 实现
// ============================================================================

RatingComponent::RatingComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 20);
}

QSize RatingComponent::sizeHint() const
{
    QFont f;
    f.setPointSize(14);
    QFontMetrics fm(f);
    int charWidth = fm.horizontalAdvance(m_starChar);
    return QSize(charWidth * m_maxValue + (m_maxValue - 1) * 2, 20);
}

void RatingComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();

    QFont f = p->font();
    f.setPointSize(14);
    p->setFont(f);

    int x = r.left();
    QFontMetrics fm(f);
    int charWidth = fm.horizontalAdvance(m_starChar);

    for (int i = 0; i < m_maxValue; ++i) {
        if (i < m_value) {
            p->setPen(m_starColor);
            p->drawText(x, r.top() + fm.ascent(), m_starChar);
        } else {
            p->setPen(m_emptyColor);
            p->drawText(x, r.top() + fm.ascent(), m_emptyChar);
        }
        x += charWidth + 2;
    }
}

// ============================================================================
// CircularProgressComponent 实现
// ============================================================================

CircularProgressComponent::CircularProgressComponent(const QString& id)
    : AbstractComponent(id)
    , m_size(40)
{
    m_sizeHint = QSize(m_size, m_size);
}

QSize CircularProgressComponent::sizeHint() const
{
    return m_sizeHint;
}

void CircularProgressComponent::paint(ComponentContext& ctx)
{
    QPainter* p = ctx.painter;
    QRect r = geometry();
    p->setRenderHint(QPainter::Antialiasing);

    int penWidth = 4;
    QRect adjusted = r.adjusted(penWidth / 2, penWidth / 2, -penWidth / 2, -penWidth / 2);

    // 绘制背景圆
    p->setPen(QPen(QColor(60, 70, 90), penWidth));
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(adjusted);

    // 绘制进度弧
    if (m_value > 0) {
        QColor progressColor = m_color.isValid() ? m_color : QColor(63, 185, 80);
        p->setPen(QPen(progressColor, penWidth, Qt::SolidLine, Qt::RoundCap));
        int spanAngle = -360 * 16 * m_value / 100;
        p->drawArc(adjusted, 90 * 16, spanAngle);
    }

    // 绘制百分比文本
    if (m_textVisible) {
        QFont f = p->font();
        f.setPointSize(r.height() / 5);
        f.setBold(true);
        p->setFont(f);
        p->setPen(ctx.textColor);
        p->drawText(r, Qt::AlignCenter, QString::number(m_value) + "%");
    }
}

// ============================================================================
// ImageComponent 实现
// ============================================================================

ImageComponent::ImageComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(100, 100);
}

QSize ImageComponent::sizeHint() const
{
    if (!m_pixmap.isNull()) {
        return m_pixmap.size();
    }
    return m_sizeHint;
}

void ImageComponent::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
}

void ImageComponent::setPath(const QString& path)
{
    m_pixmap.load(path);
}

void ImageComponent::setRounded(bool rounded, int radius)
{
    m_rounded = rounded;
    m_roundedRadius = radius;
}

void ImageComponent::paint(ComponentContext& ctx)
{
    Q_UNUSED(ctx);
    if (m_pixmap.isNull()) return;

    QPainter* p = ctx.painter;
    QRect r = geometry();
    p->setRenderHint(QPainter::Antialiasing);

    // 设置圆角裁剪
    if (m_rounded) {
        QPainterPath clipPath;
        clipPath.addRoundedRect(r, m_roundedRadius, m_roundedRadius);
        p->setClipPath(clipPath);
    }

    // 绘制缩放后的图片
    QPixmap scaled = m_pixmap.scaled(r.size(), m_aspectMode, Qt::SmoothTransformation);
    p->drawPixmap(r, scaled);

    // 恢复裁剪
    if (m_rounded) {
        p->setClipping(false);
    }
}

// ============================================================================
// CardComponent 实现
// ============================================================================

CardComponent::CardComponent(const QString& id)
    : AbstractComponent(id)
    , m_backgroundColor(45, 50, 65)
{
    m_sizeHint = QSize(200, 100);
}

QSize CardComponent::sizeHint() const
{
    return m_sizeHint;
}

void CardComponent::paint(ComponentContext& ctx)
{
    Q_UNUSED(ctx);
    QPainter* p = ctx.painter;
    QRect r = geometry();
    p->setRenderHint(QPainter::Antialiasing);

    // 绘制阴影
    if (m_shadow) {
        QRect shadowRect = r.adjusted(4, 4, 4, 4);
        p->setBrush(QColor(0, 0, 0, 50));
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(shadowRect, m_borderRadius, m_borderRadius);
    }

    // 绘制卡片背景
    p->setBrush(m_backgroundColor);
    if (m_borderWidth > 0 && m_borderColor.isValid()) {
        p->setPen(QPen(m_borderColor, m_borderWidth));
    } else {
        p->setPen(Qt::NoPen);
    }
    p->drawRoundedRect(r, m_borderRadius, m_borderRadius);
}

// ============================================================================
// SpacerComponent 实现
// ============================================================================

SpacerComponent::SpacerComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(0, 0);
}

QSize SpacerComponent::sizeHint() const
{
    return m_sizeHint;
}

void SpacerComponent::paint(ComponentContext& ctx)
{
    Q_UNUSED(ctx);
    // 占位组件不绘制任何内容
}

// ============================================================================
// ExpandArrowComponent 实现
// ============================================================================

ExpandArrowComponent::ExpandArrowComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(16, 20);
}

QSize ExpandArrowComponent::sizeHint() const
{
    return m_sizeHint;
}

void ExpandArrowComponent::paint(ComponentContext& ctx)
{
    bool hasChildren = property("hasChildren").toBool();
    if (!hasChildren) return;

    bool expanded = property("expanded").toBool();

    QPainter* p = ctx.painter;
    QRect r = geometry();
    int cx = r.center().x();
    int cy = r.center().y();
    int sz = 4;

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(QColor(170, 170, 170));
    p->setPen(Qt::NoPen);

    // 绘制三角形箭头
    QPainterPath path;
    if (expanded) {
        // 展开状态：向下的三角形
        path.moveTo(cx - sz, cy - sz / 2);
        path.lineTo(cx + sz, cy - sz / 2);
        path.lineTo(cx, cy + sz);
    } else {
        // 折叠状态：向右的三角形
        path.moveTo(cx - sz / 2, cy - sz);
        path.lineTo(cx + sz / 2 + 1, cy);
        path.lineTo(cx - sz / 2, cy + sz);
    }
    path.closeSubpath();
    p->drawPath(path);
    p->restore();
}

// ============================================================================
// DecorationIconComponent 实现
// ============================================================================

DecorationIconComponent::DecorationIconComponent(const QString& id)
    : AbstractComponent(id)
{
    m_sizeHint = QSize(20, 20);
}

QSize DecorationIconComponent::sizeHint() const
{
    return m_sizeHint;
}

void DecorationIconComponent::paint(ComponentContext& ctx)
{
    QVariant iconVar = property("icon");
    if (!iconVar.isValid()) return;

    QIcon icon = iconVar.value<QIcon>();
    if (icon.isNull()) return;

    QRect r = geometry();
    int iconSize = qMin(r.width(), r.height()) - 4;
    if (iconSize <= 0) return;

    int x = r.left() + (r.width() - iconSize) / 2;
    int y = r.top() + (r.height() - iconSize) / 2;
    QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize));
    ctx.painter->drawPixmap(x, y, pixmap);
}

} // namespace VLayout
