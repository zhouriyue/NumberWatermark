#include "hometitlebar.h"
#include "ui_hometitlebar.h"
#include "widget/underline/underlinewidget.h"

HomeTitleBar::HomeTitleBar(QString username,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomeTitleBar)
{
    this->username = username;
    ui->setupUi(this);

    // 创建头像、用户名标签和按钮
    //QLabel *avatarLabel = new QLabel(this);
    avatarLabel = new RoundImageLabel(this);
    avatarLabel->setAvatar(":/MyResource/image/person.png");  // 默认头像路径
    avatarLabel->setFixedSize(30, 30);

    userNameLabel = new QPushButton(username, this);
    userNameLabel->setFixedSize(100, 30);  // 设置大小
    int maxWidth = userNameLabel->maximumWidth() - 10;  // 10是为了给标签留出一些边距
    // 如果文本宽度超过最大宽度，则添加省略号
     QFontMetrics fm(username);
    if (fm.width(username) > maxWidth) {
        username = fm.elidedText(username, Qt::ElideRight, maxWidth);
     }
    // 设置文本居左显示
    //userNameLabel->setAlignment(Qt::AlignLeft);
    // 设置省略号后的文本
    userNameLabel->setText(username);
    userNameLabel->setStyleSheet("background: transparent; border: none; color: black;");  // 设置透明背景和文本颜色

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
    hLayout->addWidget(avatarLabel);
    hLayout->addWidget(userNameLabel);
    //hLayout->addSpacerItem(new QSpacerItem(30, 0, QSizePolicy::Fixed, QSizePolicy::Minimum));  // 设置20px的间距
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
    connect(minButton, &QPushButton::clicked, this, &HomeTitleBar::minimizeWindow);
    connect(maxButton, &QPushButton::clicked, this, &HomeTitleBar::maximizeWindow);
    connect(closeButton, &QPushButton::clicked, this, &HomeTitleBar::closeWindow);
    connect(avatarLabel, &QPushButton::clicked, this, &HomeTitleBar::avatorClick);
    connect(userNameLabel, &QPushButton::clicked, this, &HomeTitleBar::avatorClick);
}

void HomeTitleBar::avatorClick(){

    emit  avatorClickCallback();

}

// 标题栏按钮事件
void HomeTitleBar::minimizeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
        mainWin->showMinimized();
     }
}

void HomeTitleBar::maximizeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
        if (mainWin->isMaximized()) {
           mainWin->showNormal();
         } else {
           mainWin->showMaximized();
         }
     }
}

void HomeTitleBar::closeWindow() {
     QMainWindow *mainWin = qobject_cast<QMainWindow *>(parentWidget());
     if (mainWin) {
         mainWin->close();
     }
}

HomeTitleBar::~HomeTitleBar()
{
    delete ui;
    delete window;
}
