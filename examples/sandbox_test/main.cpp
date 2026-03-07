/**
 * VLayout 直接调用测试
 *
 * 测试水平布局：32x32 图标 + 拉伸文本
 */

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QLabel>
#include <vlayout/boxlayout.h>
#include <vlayout/widgetitem.h>

using namespace VLayout;

/// 简单的绘制区域，显示布局结果
class LayoutPreview : public QWidget
{
public:
    LayoutPreview(QWidget* parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("VLayout 直接调用测试");
        resize(400, 200);

        // 创建水平布局
        m_layout = std::make_shared<HBoxLayout>();
        m_layout->setContentsMargins(10, 10, 10, 10);
        m_layout->setSpacing(8);

        // 添加 32x32 图标项（固定尺寸，垂直居中）
        auto icon = std::make_shared<WidgetItem>("icon");
        icon->setSizeHint(QSize(32, 32));
        icon->setMinimumSize(QSize(32, 32));
        icon->setMaximumSize(QSize(32, 32));
        icon->setAlignment(Qt::AlignVCenter);  // 垂直居中
        m_layout->addItem(icon);

        // 添加文本项（拉伸，垂直居中）
        auto text = std::make_shared<WidgetItem>("text");
        text->setSizeHint(QSize(100, 24));  // 首选宽度 100，高度 24
        text->setMinimumSize(QSize(50, 24)); // 最小宽度 50
        text->setMaximumSize(QSize(1000000, 32)); // 最大高度 32
        text->setStretch(1);  // 拉伸因子
        text->setAlignment(Qt::AlignVCenter);  // 垂直居中
        m_layout->addItem(text);

        // 执行布局
        computeLayout();
    }

    void computeLayout()
    {
        if (!m_layout) return;

        // 设置布局区域（整个窗口）
        QRect rect(0, 0, width(), height());
        m_layout->setGeometry(rect);
        m_layout->activate();

        update();
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        computeLayout();
    }

    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        painter.fillRect(rect(), QColor(245, 245, 245));

        // 绘制容器边框
        painter.setPen(QPen(Qt::gray, 1));
        painter.drawRect(rect().adjusted(0, 0, -1, -1));

        // 绘制布局区域
        int ml = 10, mt = 10, mr = 10, mb = 10;
        QRect layoutArea(ml, mt, width() - ml - mr, height() - mt - mb);
        painter.setPen(QPen(Qt::DashLine));
        painter.drawRect(layoutArea);

        if (!m_layout) return;

        // 绘制每个布局项
        for (int i = 0; i < m_layout->count(); ++i) {
            auto item = m_layout->itemAt(i);
            if (!item) continue;

            QRect r = item->finalRect();

            // 根据类型选择颜色
            QColor fillColor;
            if (item->stretch() > 0) {
                fillColor = QColor(33, 150, 243, 150);  // 蓝色 - 拉伸项
            } else {
                fillColor = QColor(76, 175, 80, 150);    // 绿色 - 固定项
            }

            painter.fillRect(r, fillColor);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(r);

            // 获取 ID（WidgetItem 才有 id）
            QString itemId = QString("item%1").arg(i);
            if (auto* widget = dynamic_cast<WidgetItem*>(item.get())) {
                itemId = widget->id();
            }

            // 绘制标签
            QString label = QString("%1\n%2x%3")
                .arg(item->stretch() > 0 ? QString("%1 (stretch)").arg(itemId) : itemId)
                .arg(r.width())
                .arg(r.height());
            painter.drawText(r, Qt::AlignCenter, label);
        }

        // 绘制信息
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setPointSize(10);
        painter.setFont(font);
        painter.drawText(10, height() - 10,
            QString("容器: %1x%2 | 图标: 32x32 固定 | 文本: stretch=1").arg(width()).arg(height()));
    }

private:
    std::shared_ptr<BoxLayout> m_layout;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto preview = new LayoutPreview();
    preview->show();

    return app.exec();
}
