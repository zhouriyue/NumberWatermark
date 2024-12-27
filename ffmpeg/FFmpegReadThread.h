#ifndef FFMPEGREADTHREAD_H
#define FFMPEGREADTHREAD_H
#include <QtWidgets>
#include "FFmpegDecode.h"

class FFmpegReadThread : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegReadThread(QObject *parent = nullptr);
    ~FFmpegReadThread() override;

    bool openMedia(const QString& strUrl);              // 打开视频
    void closeMedia();                                  // 关闭视频
    const QString& url();                               // 获取打开的视频地址
    const qint64& getTotalFrames();                     // 获取视频总帧数
    const QSize& getVideoSize();                        // 获取视频尺寸

protected:
    void run() override;

signals:
    void repaint(uchar * pYUVData, int nWidth, int nHeight);    // 重绘
    void readEnd();                                             // 读取结束信号

private:
    FFmpegDecode* m_ffmpegDecode = nullptr;                     // ffmpeg解码类
    QString m_strUrl;                                           // 打开的视频地址
    qint64 m_nTotalFrames;                                      // 视频总帧数
    QSize  m_size;                                              // 视频分辨率大小
};

#endif // FFMPEGREADTHREAD_H
