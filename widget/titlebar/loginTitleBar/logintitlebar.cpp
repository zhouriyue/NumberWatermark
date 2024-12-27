#include "logintitlebar.h"
#include "ui_logintitlebar.h"

LoginTitleBar::LoginTitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginTitleBar)
{
    ui->setupUi(this);

    QPushButton *minButton = new QPushButton("-", this);
    QPushButton *maxButton = new QPushButton("□", this);
    QPushButton *closeButton = new QPushButton("X", this);

    // 设置按钮大小
    minButton->setFixedSize(30, 30);
    maxButton->setFixedSize(30, 30);
    closeButton->setFixedSize(30, 30);

    // 布局：将头像、用户名和按钮放在一行
    QHBoxLayout *hLayout = new QHBoxLayout();


    hLayout->addStretch(); // 拉伸，使按钮右对齐
    hLayout->addSpacerItem(new QSpacerItem(30, 0, QSizePolicy::Fixed, QSizePolicy::Minimum));  // 设置20px的间距
    hLayout->addWidget(minButton);
    hLayout->addWidget(maxButton);
    hLayout->addWidget(closeButton);

    // 创建垂直布局：将标题栏和下划线添加进去
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout); // 添加标题栏
    // 创建下划线
    QFrame *underline = new QFrame(this);
    underline->setFrameShape(QFrame::HLine);  // 设置为水平线
    underline->setFrameShadow(QFrame::Sunken);  // 设置阴影效果

    // 设置下划线颜色，通过样式表修改
    underline->setStyleSheet("QFrame { border: 0; background-color: #ffffff; }");
    vLayout->addWidget(underline); // 添加下划线

    // 设置布局
    setLayout(vLayout);

    // 连接按钮事件
    connect(minButton, &QPushButton::clicked, this, &LoginTitleBar::minimizeWindow);
    connect(maxButton, &QPushButton::clicked, this, &LoginTitleBar::maximizeWindow);
    connect(closeButton, &QPushButton::clicked, this, &LoginTitleBar::closeWindow);
}

LoginTitleBar::~LoginTitleBar()
{
    delete ui;
    delete window;
}


// 标题栏按钮事件
void LoginTitleBar::minimizeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
        mainWin->showMinimized();
     }
}

void LoginTitleBar::maximizeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
        if (mainWin->isMaximized()) {
           mainWin->showNormal();
         } else {
           mainWin->showMaximized();
         }
     }
}

void LoginTitleBar::closeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
         mainWin->close();
     }
}
