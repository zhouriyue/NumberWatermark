#ifndef FFMPEGREAD_H
#define FFMPEGREAD_H

#include "ffmpeg/FFmpegInclude.h"
//#include "FFmpegWrite.h"

#include <string>

using namespace std;

class FFmpegRead : public QThread{
    Q_OBJECT

public:
    explicit FFmpegRead(QObject *parent = nullptr);
    ~FFmpegRead() override;
    void freeObject();
    int decodeFrame();//解码出视频帧
    int openFile(const string filePath);//1.打开文件
    void stopRead();//停止读循环

    //辅助方法
    qreal rationalToDouble(AVRational* rational);
    void waitDts();
    void setNeedEnd(bool isNeedEnd){
        this->isNeedEnd = isNeedEnd;
    }
    QPixmap avFrameToQPixmap(AVFrame *frame);



    AVFormatContext * m_inputFormatContext  = nullptr;      // 输入格式上下文
    AVCodecContext *  m_videoCodecCtx       = nullptr;      // 视频解码器上下文
    AVCodecContext *  m_audioCodecCtx       = nullptr;      // 音频解码器上下文
    string filePath;                                        // 文件路径
    int m_videoIndex = 0;                                   // 获取视频流ID
    int m_audioIndex = 0;                                   // 获取音频流ID
    QSize  m_size;                                          // 视频分辨率大小
    int m_fps        = 0;                                   // 帧率
    int m_totalFrames;                                       // 总帧数
    AVPacket * m_packet                     = nullptr;      // 解码前的数据包
    int m_dts        = 0;                                   // 视频帧的解码时间戳
    qint64 m_startTime = 0;                                 // 开始解码的时间
    int m_frameCount = 0;                                   // 帧数
    bool isNeedEnd = false;                                 // 是否需要结束
    AVRational m_timeBase;                                  // 视频时间基准

signals:
    void readIndex(int type,QPixmap pixmapFrame,AVPacket* audioData,int ret);//读取
    void readEnd();
    void readError();

protected:
    void run() override;//重写run函数

};

#endif // FFMPEGREAD_H
