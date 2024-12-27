#include "myplayer.h"
#include "ui_myplayer.h"

MyPlayer::MyPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyPlayer)
{
    ui->setupUi(this);
    // 创建视频播放控件和播放器
    player = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(this);

   // 设置视频输出到 QVideoWidget
   player->setVideoOutput(videoWidget);

    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(videoWidget);
    //layout->addWidget(playButton);

     // 设置窗口标题
     setWindowTitle("Video Player");

     // 设置默认窗口大小
     resize(640, 480);

     // 获取屏幕的尺寸
     QScreen *screen = QGuiApplication::primaryScreen();
     QRect screenGeometry = screen->geometry();

     // 获取当前窗口的尺寸
     QRect windowGeometry = geometry();

     // 计算窗口的左上角坐标，使其居中
     int x = (screenGeometry.width() - windowGeometry.width()) / 2;
     int y = (screenGeometry.height() - windowGeometry.height()) / 2;

     // 移动窗口
     move(x, y);
}

MyPlayer::~MyPlayer()
{
    delete ui;
}


void MyPlayer::playVideo(QString url){
    this->url = url;
    // 设置视频源并开始播放
    player->setMedia(QUrl::fromLocalFile(url));
    player->play();

    // 等待视频信息加载完成
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MyPlayer::onMediaStatusChanged);
}
