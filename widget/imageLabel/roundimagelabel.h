#ifndef ROUNDIMAGELABEL_H
#define ROUNDIMAGELABEL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QImage>

namespace Ui {
class RoundImageLabel;
}

class RoundImageLabel : public QPushButton
{
    Q_OBJECT

public:
    explicit RoundImageLabel(QWidget *parent = nullptr);
    ~RoundImageLabel();

    // 设置头像图片
    void setAvatar(const QString &imagePath)
    {
        QPixmap pixmap(imagePath);
        avatarPixmap = pixmap;
        update();  // 图片更新后调用 update() 来触发重绘
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿，确保绘制平滑

        if (!avatarPixmap.isNull()) {
            // 创建一个圆形的裁剪区域
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            // 设置裁剪区域为圆形
            QPainterPath path;
            path.addEllipse(0, 0, width(), height());
            painter.setClipPath(path);

            // 将头像图片缩放到当前控件大小并绘制圆形
            painter.drawPixmap(0, 0, avatarPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

private:
    Ui::RoundImageLabel *ui;
    QPixmap avatarPixmap;
};

#endif // ROUNDIMAGELABEL_H
