#ifndef MYPLAYER_H
#define MYPLAYER_H

#include <QApplication>
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QScreen>


namespace Ui {
class MyPlayer;
}

class MyPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MyPlayer(QWidget *parent = nullptr);
    ~MyPlayer();
    void playVideo(QString url);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            // 获取视频的宽度和高度
            QVariant widthVariant = player->metaData("VideoWidth");
            QVariant heightVariant = player->metaData("VideoHeight");

            if (widthVariant.isValid() && heightVariant.isValid()) {
                int width = widthVariant.toInt();
                int height = heightVariant.toInt();

                qDebug() << "Video Width:" << width << "Height:" << height;

                // 根据视频分辨率设置窗口大小
                if (width > 0 && height > 0) {
                    resize(width, height);  // 设置窗口大小为视频的分辨率
                }
            } else {
                qWarning() << "Failed to retrieve video width or height";
            }
        }
    }

private:
    Ui::MyPlayer *ui;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
    QString url;
};

#endif // MYPLAYER_H
