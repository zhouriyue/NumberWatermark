#ifndef LOGINTITLEBAR_H
#define LOGINTITLEBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMouseEvent>

namespace Ui {
class LoginTitleBar;
}

class LoginTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit LoginTitleBar(QWidget *parent = nullptr);
    ~LoginTitleBar();

    void setTitle(const QString &title);

    void minimizeWindow();

    void maximizeWindow();

    void closeWindow();

    void setWindow(QMainWindow *window){
        this->window = window;
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        // 只有左键按下时才处理拖动
        if (event->button() == Qt::LeftButton) {
            // 记录鼠标按下时的位置
            dragPosition = event->globalPos() - parentWidget()->frameGeometry().topLeft();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        // 如果左键按下，并且移动鼠标，则拖动窗口
        if (event->buttons() & Qt::LeftButton) {
            parentWidget()->move(event->globalPos() - dragPosition);
            event->accept();
        }
    }

private:
    QPoint dragPosition;  // 鼠标按下时的相对位置
    Ui::LoginTitleBar *ui;
    QMainWindow *window;
};

#endif // LOGINTITLEBAR_H
