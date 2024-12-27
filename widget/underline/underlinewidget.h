#ifndef UNDERLINEWIDGET_H
#define UNDERLINEWIDGET_H

#include <QWidget>
#include <QPainter>

namespace Ui {
class UnderlineWidget;
}

class UnderlineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UnderlineWidget(QWidget *parent = nullptr);
    ~UnderlineWidget();

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setPen(QPen(Qt::black, 2));  // 设置黑色、宽度为2的画笔
        painter.drawLine(0, height() - 7, width(), height() - 7);  // 绘制下划线
    }

private:
    Ui::UnderlineWidget *ui;
};

#endif // UNDERLINEWIDGET_H
