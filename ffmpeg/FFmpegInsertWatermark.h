#ifndef FFMPEGINSERTWATERMARK_H
#define FFMPEGINSERTWATERMARK_H

#include <malloc.h>
#include <QtWidgets>
#include "FFmpegInclude.h"
#include "watermark/Dct.h"
#include <windows.h>
#include <iostream>

class FFmpegInsertWatermark : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegInsertWatermark(QObject *parent = nullptr);
    ~FFmpegInsertWatermark() override;

    //比较水印
    bool compareWatermark(AVFrame* frame);

    //嵌入水印
    int insertWatermark(AVFrame *frame,Mat*& outMat);

    ///
    /// \brief 打开输入文件
    /// \param pFilename 文件名
    /// \return 成功返回ture；失败返回false
    ///
    bool openInputFile(const char* pFilename);

    ///
    /// \brief 创建输出文件
    /// \param pFilename 文件名
    /// \return 成功返回true；失败返回false
    ///
    bool createOutputFile(const char* pFilename);

    ///
    /// \brief 停止
    ///
    void stopInsert();
    Dct* dct = nullptr;                                     // 数字水印类
    bool isInsert = true;                                  // 功能类型：嵌入、提取


protected:
    void run() override;

private:

    ///
    /// \brief 初始化
    ///
    bool initObject();

    ///
    /// \brief 清除
    ///
    void clearObject();

    ///
    /// \brief 释放
    ///
    void freeObject();

    ///
    /// \brief 查找流
    /// \return 成功返回0；失败返回错误码
    ///
    int findStream();

    ///
    /// \brief 打开解码器
    /// \return 成功返回0；失败返回错误码
    ///
    int openDecoder();

    ///
    /// \brief 初始化编码器
    /// \return
    ///
    int initEncoderCodec();

    ///
    /// \brief 视频解码处理
    /// \return
    ///
    int decodeVideoFrame();

    ///
    /// \brief 写入视频文件头
    /// \return 成功返回0；失败返回错误码
    ///
    int writeHeader(const char* pFilename);

    ///
    /// \brief 写入视频帧
    /// \param pFrame 要写入的视频帧
    /// \return 成功返回0；失败返回错误码
    ///
    int writeFrame(AVFrame* pFrame);

    ///
    /// \brief 写入视频包
    /// \param packet 要写入的视频包
    /// \param nIndex 要写入的视频包的索引
    /// \return 成功返回0；失败返回错误码
    ///
    int writePacket(AVPacket * packet, int nIndex);

    ///
    /// \brief 写入视频尾
    /// \return 成功返回0；失败返回错误码
    ///
    int writeTrailer();

    ///
    /// \brief 等待当前帧的应解码时间(dts)
    ///
    void waitDts();

    ///
    /// \brief 将AVRational转换为double
    /// \param rational
    /// \return
    ///
    qreal rationalToDouble(AVRational* rational);

signals:
    void progress(int current);
    void signalEnd();
    void signalError();

private:
    AVFormatContext * m_inputFormatContext  = nullptr;      // 输入格式上下文
    AVFormatContext * m_outputFormatContext = nullptr;      // 输出格式上下文
    AVCodecContext *  m_videoCodecCtx       = nullptr;      // 视频解码器上下文
    AVCodecContext *  m_videoEnCodecCtx     = nullptr;      // 视频编码器上下文
    AVCodecContext *  m_audioCodecCtx       = nullptr;      // 音频解码器上下文
    AVCodec *         m_videoEnCodec        = nullptr;      // 视频编码器
    AVPacket * m_packet                     = nullptr;      // 解码前的数据包
    //uchar*     m_bufferVideo                = nullptr;      // 视频帧的缓存数据
    QSize  m_size;                                          // 视频分辨率大小
    int m_videoIndex = 0;                                   // 视频流索引
    int m_audioIndex = 0;                                   // 音频流索引
    int m_fps        = 0;                                   // 帧率
    int m_dts        = 0;                                   // 视频帧的解码时间戳
    int m_frameCount = 0;                                   // 帧数
    AVRational m_timeBase;                                  // 视频时间基准
    qint64 m_startTime = 0;                                 // 开始解码的时间
    char* m_filename;                                        // 文件

    int success = 0;
    int fail = 0;
    int nI = 0;
    int nP = 0;
    int nB = 0;
    int nO = 0;
};

AVFrame* matToAvFrame(const cv::Mat& mat);

#endif // FFMPEGINSERTWATERMARK_H
