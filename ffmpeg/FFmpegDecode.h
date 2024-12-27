#ifndef FFMPEGDECODE_H
#define FFMPEGDECODE_H

#include "FFmpegInclude.h"
#include <QSize>

class FFmpegDecode
{
public:
    explicit FFmpegDecode(bool bIsEnableVideo = true, bool bIsEnableAudio = true);
    ~FFmpegDecode();

    bool openMedia(const QString& strUrl = QString()); // 打开媒体文件
    bool readFrame(uchar * pYUVData, int * nWidth, int * nHeight);// 读取帧
    void closeMedia();                                 // 关闭
    bool isEnd();                                      // 是否读取完成
    const qint64& pts();                               // 获取当前帧显示时间
    const qint64& dts();                               // 获取当前帧解码时间
    const int& getFrameBufferSize();                   // 获取视频帧数据大小
    const qint64& getTotalFrames();                    // 获取视频总帧数
    const QSize& getVideoSize();                       // 获取视频尺寸

private:
    bool initObject();                                 // 初始化对象
    qreal rationalToDouble(AVRational* rational);      // 将AVRational转换为double
    void clearObject();                                // 清空读取缓冲
    void freeObject();                                 // 释放

private:
    bool m_bIsEnableVideo = true;                      // 是否启用视频
    bool m_bIsEnableAudio = true;                      // 是否启用音频
    AVFormatContext* m_formatContext = nullptr;        // 解封装上下文
    AVCodecContext*  m_codecContext  = nullptr;        // 解码器上下文
    SwsContext*      m_swsContext    = nullptr;        // 图像转换上下文
    AVPacket* m_packet = nullptr;                      // 数据包
    AVFrame*  m_frame  = nullptr;                      // 解码后的视频帧
    int    m_videoIndex   = 0;                         // 视频流索引
    qint64 m_totalTime    = 0;                         // 视频总时长
    qint64 m_totalFrames  = 0;                         // 视频总帧数
    qint64 m_pts          = 0;                         // 图像帧的显示时间
    qint64 m_dts          = 0;                         // 图像帧的解码时间
    qreal  m_frameRate    = 0;                         // 视频帧率
    QSize  m_size;                                     // 视频分辨率大小
    bool   m_end = false;                              // 视频读取完成
    uchar* m_bufferVideo = nullptr;                    // 视频帧的缓存数据
    int    m_bufferSizeVido = 0;                       // 视频帧的缓存数据大小
};

#endif // FFMPEGDECODE_H
