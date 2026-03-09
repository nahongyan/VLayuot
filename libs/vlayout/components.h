#ifndef VLAYOUT_COMPONENTS_H
#define VLAYOUT_COMPONENTS_H

/**
 * @file components.h
 * @brief 具体组件类定义
 *
 * 提供各种常用的 UI 组件实现，包括标签、按钮、复选框、进度条等。
 * 所有组件都继承自 AbstractComponent，可用于 Delegate 中的自定义绘制。
 */

#include "vlayout/component.h"
#include <QFont>
#include <QPixmap>
#include <QIcon>
#include <QPainterPath>

namespace VLayout {

// ============================================================================
// LabelComponent - 文本标签组件
// ============================================================================

/**
 * @class LabelComponent
 * @brief 文本标签组件，用于显示文本内容
 *
 * 支持文本对齐、自动换行和省略模式。
 */
class VLAYOUT_EXPORT LabelComponent : public AbstractComponent
{
public:
    explicit LabelComponent(const QString& id);

    QString type() const override { return "Label"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置文本内容
    void setText(const QString& text) { setProperty("text", text); }

    /// 获取文本内容
    QString text() const { return property("text").toString(); }

    /// 设置对齐方式
    void setAlignment(Qt::Alignment align) { m_alignment = align; }

    /// 获取对齐方式
    Qt::Alignment alignment() const { return m_alignment; }

    /// 设置是否自动换行
    void setWordWrap(bool wrap) { m_wordWrap = wrap; }

    /// 设置省略模式
    void setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; }

private:
    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool m_wordWrap = false;
    Qt::TextElideMode m_elideMode = Qt::ElideRight;
};

// ============================================================================
// ButtonComponent - 按钮组件
// ============================================================================

/**
 * @class ButtonComponent
 * @brief 按钮组件，支持文本、图标和可选中状态
 */
class VLAYOUT_EXPORT ButtonComponent : public AbstractComponent
{
public:
    explicit ButtonComponent(const QString& id);

    QString type() const override { return "Button"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
    void onClick(const QPoint& pos) override;

    /// 设置按钮文本
    void setText(const QString& text) { m_text = text; }

    /// 获取按钮文本
    QString text() const { return m_text; }

    /// 设置图标（图标字体字符）
    void setIcon(const QString& icon) { m_icon = icon; }

    /// 获取图标
    QString icon() const { return m_icon; }

    /// 设置是否可选中
    void setCheckable(bool checkable) { m_checkable = checkable; }

    /// 是否可选中
    bool isCheckable() const { return m_checkable; }

    /// 设置是否扁平样式
    void setFlat(bool flat) { m_flat = flat; }

private:
    QString m_text;
    QString m_icon;
    bool m_checkable = false;
    bool m_flat = false;
};

// ============================================================================
// CheckBoxComponent - 复选框组件
// ============================================================================

/**
 * @class CheckBoxComponent
 * @brief 复选框组件，支持勾选状态
 */
class VLAYOUT_EXPORT CheckBoxComponent : public AbstractComponent
{
public:
    explicit CheckBoxComponent(const QString& id);

    QString type() const override { return "CheckBox"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
    void onClick(const QPoint& pos) override;

    /// 设置文本
    void setText(const QString& text) { m_text = text; }

    /// 获取文本
    QString text() const { return m_text; }

private:
    QString m_text;
};

// ============================================================================
// ProgressBarComponent - 进度条组件
// ============================================================================

/**
 * @class ProgressBarComponent
 * @brief 进度条组件，显示进度值
 */
class VLAYOUT_EXPORT ProgressBarComponent : public AbstractComponent
{
public:
    explicit ProgressBarComponent(const QString& id);

    QString type() const override { return "ProgressBar"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置当前值
    void setValue(int value) { m_value = value; }

    /// 获取当前值
    int value() const { return m_value; }

    /// 设置范围
    void setRange(int min, int max) { m_min = min; m_max = max; }

    /// 设置是否显示文本
    void setTextVisible(bool visible) { m_textVisible = visible; }

private:
    int m_value = 0;
    int m_min = 0;
    int m_max = 100;
    bool m_textVisible = true;
};

// ============================================================================
// IconComponent - 图标组件
// ============================================================================

/**
 * @class IconComponent
 * @brief 图标组件，显示图标字体字符或纯色块
 */
class VLAYOUT_EXPORT IconComponent : public AbstractComponent
{
public:
    explicit IconComponent(const QString& id);

    QString type() const override { return "Icon"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置图标（图标字体字符）
    void setIcon(const QString& icon) { m_icon = icon; }

    /// 获取图标
    QString icon() const { return m_icon; }

    /// 设置背景颜色
    void setColor(const QColor& color) { m_color = color; }

    /// 获取背景颜色
    QColor color() const { return m_color; }

    /// 设置图标尺寸
    void setSize(const QSize& size) { m_iconSize = size; }

private:
    QString m_icon;
    QColor m_color;
    QSize m_iconSize = QSize(24, 24);
};

// ============================================================================
// BadgeComponent - 徽章组件
// ============================================================================

/**
 * @class BadgeComponent
 * @brief 徽章组件，显示小数字或文本标记
 */
class VLAYOUT_EXPORT BadgeComponent : public AbstractComponent
{
public:
    explicit BadgeComponent(const QString& id);

    QString type() const override { return "Badge"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置文本
    void setText(const QString& text) { m_text = text; }

    /// 获取文本
    QString text() const { return m_text; }

    /// 设置背景颜色
    void setColor(const QColor& color) { m_color = color; }

private:
    QString m_text;
    QColor m_color;
};

// ============================================================================
// SeparatorComponent - 分隔线组件
// ============================================================================

/**
 * @class SeparatorComponent
 * @brief 分隔线组件，用于分隔内容区域
 */
class VLAYOUT_EXPORT SeparatorComponent : public AbstractComponent
{
public:
    explicit SeparatorComponent(const QString& id);

    QString type() const override { return "Separator"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置方向
    void setOrientation(Qt::Orientation orient) { m_orientation = orient; }

private:
    Qt::Orientation m_orientation = Qt::Horizontal;
};

// ============================================================================
// AvatarComponent - 头像组件
// ============================================================================

/**
 * @class AvatarComponent
 * @brief 头像组件，显示圆形头像或首字母
 */
class VLAYOUT_EXPORT AvatarComponent : public AbstractComponent
{
public:
    explicit AvatarComponent(const QString& id);

    QString type() const override { return "Avatar"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置首字母文本
    void setText(const QString& text) { m_text = text; }

    /// 设置头像图片
    void setImage(const QPixmap& pixmap) { m_pixmap = pixmap; }

    /// 设置头像尺寸
    void setSize(int size) { m_size = size; }

private:
    QString m_text;
    QPixmap m_pixmap;
    int m_size = 40;
};

// ============================================================================
// SwitchComponent - 开关组件
// ============================================================================

/**
 * @class SwitchComponent
 * @brief 开关组件，类似于滑动开关
 */
class VLAYOUT_EXPORT SwitchComponent : public AbstractComponent
{
public:
    explicit SwitchComponent(const QString& id);

    QString type() const override { return "Switch"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
    void onClick(const QPoint& pos) override;

    /// 设置开关状态
    void setOn(bool on) { setState(ComponentState::Checked, on); }

    /// 获取开关状态
    bool isOn() const { return isChecked(); }

private:
    void drawThumb(QPainter* p, const QRect& rect, bool on, const QColor& color);
};

// ============================================================================
// SliderComponent - 滑动条组件
// ============================================================================

/**
 * @class SliderComponent
 * @brief 滑动条组件，支持水平和垂直方向
 */
class VLAYOUT_EXPORT SliderComponent : public AbstractComponent
{
public:
    explicit SliderComponent(const QString& id);

    QString type() const override { return "Slider"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置当前值
    void setValue(int value) { m_value = value; }

    /// 获取当前值
    int value() const { return m_value; }

    /// 设置范围
    void setRange(int min, int max) { m_min = min; m_max = max; }

    /// 设置方向
    void setOrientation(Qt::Orientation orient) { m_orientation = orient; }

private:
    int m_value = 0;
    int m_min = 0;
    int m_max = 100;
    Qt::Orientation m_orientation = Qt::Horizontal;
};

// ============================================================================
// ComboBoxComponent - 下拉框组件
// ============================================================================

/**
 * @class ComboBoxComponent
 * @brief 下拉框组件，显示选项列表
 */
class VLAYOUT_EXPORT ComboBoxComponent : public AbstractComponent
{
public:
    explicit ComboBoxComponent(const QString& id);

    QString type() const override { return "ComboBox"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
    void onClick(const QPoint& pos) override;

    /// 设置选项列表
    void setItems(const QStringList& items) { m_items = items; }

    /// 获取选项列表
    QStringList items() const { return m_items; }

    /// 设置当前索引
    void setCurrentIndex(int index);

    /// 获取当前索引
    int currentIndex() const { return m_currentIndex; }

    /// 获取当前文本
    QString currentText() const;

private:
    QStringList m_items;
    int m_currentIndex = -1;
    bool m_dropDownVisible = false;
};

// ============================================================================
// SpinBoxComponent - 调节框组件
// ============================================================================

/**
 * @class SpinBoxComponent
 * @brief 整数调节框组件
 */
class VLAYOUT_EXPORT SpinBoxComponent : public AbstractComponent
{
public:
    explicit SpinBoxComponent(const QString& id);

    QString type() const override { return "SpinBox"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
    void onClick(const QPoint& pos) override;

    /// 设置当前值
    void setValue(int value) { m_value = qBound(m_min, value, m_max); }

    /// 获取当前值
    int value() const { return m_value; }

    /// 设置范围
    void setRange(int min, int max) { m_min = min; m_max = max; }

    /// 设置步进值
    void setStep(int step) { m_step = step; }

    /// 设置后缀
    void setSuffix(const QString& suffix) { m_suffix = suffix; }

private:
    int m_value = 0;
    int m_min = 0;
    int m_max = 99;
    int m_step = 1;
    QString m_suffix;
};

// ============================================================================
// RatingComponent - 评分组件
// ============================================================================

/**
 * @class RatingComponent
 * @brief 评分组件，显示星级评分
 */
class VLAYOUT_EXPORT RatingComponent : public AbstractComponent
{
public:
    explicit RatingComponent(const QString& id);

    QString type() const override { return "Rating"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置评分值
    void setValue(int value) { m_value = value; }

    /// 获取评分值
    int value() const { return m_value; }

    /// 设置最大值
    void setMaxValue(int max) { m_maxValue = max; }

    /// 设置填充星字符
    void setStarChar(const QChar& ch) { m_starChar = ch; }

    /// 设置空心星字符
    void setEmptyChar(const QChar& ch) { m_emptyChar = ch; }

    /// 设置填充星颜色
    void setStarColor(const QColor& c) { m_starColor = c; }

    /// 设置空心星颜色
    void setEmptyColor(const QColor& c) { m_emptyColor = c; }

private:
    int m_value = 0;
    int m_maxValue = 5;
    QChar m_starChar = QChar(0x2605);   // 填充星
    QChar m_emptyChar = QChar(0x2606);  // 空心星
    QColor m_starColor = QColor(255, 180, 50);
    QColor m_emptyColor = QColor(100, 110, 130);
};

// ============================================================================
// CircularProgressComponent - 圆形进度组件
// ============================================================================

/**
 * @class CircularProgressComponent
 * @brief 圆形进度指示器组件
 */
class VLAYOUT_EXPORT CircularProgressComponent : public AbstractComponent
{
public:
    explicit CircularProgressComponent(const QString& id);

    QString type() const override { return "CircularProgress"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置进度值（0-100）
    void setValue(int value) { m_value = qBound(0, value, 100); }

    /// 获取进度值
    int value() const { return m_value; }

    /// 设置尺寸
    void setSize(int size) { m_size = size; m_sizeHint = QSize(size, size); }

    /// 设置颜色
    void setColor(const QColor& c) { m_color = c; }

    /// 设置是否显示百分比文本
    void setTextVisible(bool visible) { m_textVisible = visible; }

private:
    int m_value = 0;
    int m_size = 40;
    QColor m_color;
    bool m_textVisible = true;
    QSize m_sizeHint;
};

// ============================================================================
// ImageComponent - 图片组件
// ============================================================================

/**
 * @class ImageComponent
 * @brief 图片组件，显示 QPixmap 图片
 */
class VLAYOUT_EXPORT ImageComponent : public AbstractComponent
{
public:
    explicit ImageComponent(const QString& id);

    QString type() const override { return "Image"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置图片
    void setPixmap(const QPixmap& pixmap);

    /// 从文件路径加载图片
    void setPath(const QString& path);

    /// 设置缩放模式
    void setMode(Qt::AspectRatioMode mode) { m_aspectMode = mode; }

    /// 设置是否圆角
    void setRounded(bool rounded, int radius = 0);

private:
    QPixmap m_pixmap;
    Qt::AspectRatioMode m_aspectMode = Qt::KeepAspectRatioByExpanding;
    bool m_rounded = false;
    int m_roundedRadius = 0;
};

// ============================================================================
// CardComponent - 卡片组件
// ============================================================================

/**
 * @class CardComponent
 * @brief 卡片组件，带背景和圆角的容器
 */
class VLAYOUT_EXPORT CardComponent : public AbstractComponent
{
public:
    explicit CardComponent(const QString& id);

    QString type() const override { return "Card"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;

    /// 设置背景颜色
    void setBackgroundColor(const QColor& c) { m_backgroundColor = c; }

    /// 设置边框颜色
    void setBorderColor(const QColor& c) { m_borderColor = c; }

    /// 设置边框宽度
    void setBorderWidth(int w) { m_borderWidth = w; }

    /// 设置圆角半径
    void setBorderRadius(int r) { m_borderRadius = r; }

    /// 设置是否显示阴影
    void setShadow(bool shadow) { m_shadow = shadow; }

private:
    QColor m_backgroundColor;
    QColor m_borderColor;
    int m_borderWidth = 0;
    int m_borderRadius = 8;
    bool m_shadow = false;
};

// ============================================================================
// SpacerComponent - 占位组件
// ============================================================================

/**
 * @class SpacerComponent
 * @brief 占位组件，用于布局中占用空间但不绘制任何内容
 */
class VLAYOUT_EXPORT SpacerComponent : public AbstractComponent
{
public:
    explicit SpacerComponent(const QString& id);

    QString type() const override { return "Spacer"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
};

// ============================================================================
// ExpandArrowComponent - 展开箭头组件
// ============================================================================

/**
 * @class ExpandArrowComponent
 * @brief 展开/折叠箭头组件，用于树形结构
 */
class VLAYOUT_EXPORT ExpandArrowComponent : public AbstractComponent
{
public:
    explicit ExpandArrowComponent(const QString& id);

    QString type() const override { return "ExpandArrow"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
};

// ============================================================================
// DecorationIconComponent - 装饰图标组件
// ============================================================================

/**
 * @class DecorationIconComponent
 * @brief QIcon 装饰图标组件，显示 Qt 标准图标
 */
class VLAYOUT_EXPORT DecorationIconComponent : public AbstractComponent
{
public:
    explicit DecorationIconComponent(const QString& id);

    QString type() const override { return "DecorationIcon"; }
    QSize sizeHint() const override;
    void paint(ComponentContext& ctx) override;
};

} // namespace VLayout

#endif // VLAYOUT_COMPONENTS_H
