#include "FFmpegDecode.h"
#include <QFile>

FFmpegDecode::FFmpegDecode(bool bIsEnableVideo, bool bIsEnableAudio)
{
    m_bIsEnableVideo = bIsEnableVideo;
    m_bIsEnableAudio = bIsEnableAudio;
}

FFmpegDecode::~FFmpegDecode()
{
    closeMedia();
    if(m_bufferVideo)
    {
        free(m_bufferVideo);
        m_bufferVideo = nullptr;
    }
}

///
/// \brief     打开媒体文件
/// \param url 媒体文件
/// \return    true：成功  false：失败
///
bool FFmpegDecode::openMedia(const QString &strUrl)
{
    if(strUrl.isNull()) return false;

    // 打开输入流并返回解封装上下文
    int ret = avformat_open_input(&m_formatContext,             // 返回解封装上下文
                                  strUrl.toStdString().data(),  // 打开视频地址
                                  nullptr,                      // 如果非null，此参数强制使用特定的输入格式。自动选择解封装器（文件格式）
                                  nullptr);                     // 参数设置

    // 打开视频失败
    if(ret < 0)
    {
        freeObject();
        return false;
    }

    // 读取媒体文件的数据包以获取流信息。
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if(ret < 0)
    {
        freeObject();
        return false;
    }
    m_totalTime = m_formatContext->duration / (AV_TIME_BASE / 1000); // 计算视频总时长（毫秒）
    qDebug() << QString("视频总时长：%1 ms，[%2]").arg(m_totalTime).arg(QTime::fromMSecsSinceStartOfDay(int(m_totalTime)).toString("HH:mm:ss zzz"));

    //----------------视频流----------------
    if(m_bIsEnableVideo)
    {
        // 通过AVMediaType枚举查询视频流ID（也可以通过遍历查找），最后一个参数无用
        m_videoIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if(m_videoIndex < 0)
        {
            qWarning() << "查找视频流失败";
            freeObject();
            return false;
        }

        AVStream* videoStream = m_formatContext->streams[m_videoIndex];  // 通过查询到的索引获取视频流

        // 获取视频图像分辨率（AVStream中的AVCodecContext在新版本中弃用，改为使用AVCodecParameters）
        m_size.setWidth(videoStream->codecpar->width);
        m_size.setHeight(videoStream->codecpar->height);
        m_frameRate = rationalToDouble(&videoStream->avg_frame_rate);  // 视频帧率

        // 通过解码器ID获取视频解码器（新版本返回值必须使用const）
        const AVCodec* codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
        m_totalFrames = videoStream->nb_frames;

        qDebug() << QString("分辨率：[w:%1,h:%2] 帧率：%3  总帧数：%4  解码器：%5")
                    .arg(m_size.width()).arg(m_size.height()).arg(m_frameRate).arg(m_totalFrames).arg(codec->name);

        // 分配AVCodecContext并将其字段设置为默认值。
        m_codecContext = avcodec_alloc_context3(codec);
        if(!m_codecContext)
        {
            qWarning() << "创建视频解码器上下文失败！";
            freeObject();
            return false;
        }

        // 使用视频流的codecpar为解码器上下文赋值
        ret = avcodec_parameters_to_context(m_codecContext, videoStream->codecpar);
        if(ret < 0)
        {
            qWarning() << "视频解码器上下文赋值失败！";
            freeObject();
            return false;
        }

        //m_codecContext->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
        //m_codecContext->thread_count = 8;                 // 使用8线程解码

        // 初始化解码器上下文，如果之前avcodec_alloc_context3传入了解码器，这里设置NULL就可以
        ret = avcodec_open2(m_codecContext, nullptr, nullptr);
        if(ret < 0)
        {
            qWarning() << "初始化解码器上下文失败！";
            freeObject();
            return false;
        }
    }

    return initObject();
}

///
/// \brief 初始化需要用到的对象
/// \return
///
bool FFmpegDecode::initObject()
{
    // 分配AVPacket并将其字段设置为默认值。
    m_packet = av_packet_alloc();
    if(!m_packet)
    {
        qWarning() << "[初始化]分配视频包失败";
        freeObject();
        return false;
    }
    // 分配AVFrame并将其字段设置为默认值。
    m_frame = av_frame_alloc();
    if(!m_frame)
    {
        qWarning() << "[初始化]分配视频帧失败";
        freeObject();
        return false;
    }
    // 由于传递时是浅拷贝，可能显示类还没处理完成，所以如果播放完成就释放可能会崩溃；
    if(m_bufferVideo)
    {
        free(m_bufferVideo);
        m_bufferVideo = nullptr;
    }
    // 分配图像空间
    m_bufferSizeVido = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_size.width(), m_size.height(), 4);
    /**
     * 【注意：】这里可以多分配一些，否则如果只是安装size分配，大部分视频图像数据拷贝没有问题，
     *         但是少部分视频图像在使用sws_scale()拷贝时会超出数组长度，在使用使用msvc debug模式时delete[] m_buffer会报错
     *        （HEAP CORRUPTION DETECTED: after Normal block(#32215) at 0x000001AC442830370.CRT delected that the application wrote to memory after end of heap buffer）
     */
    m_bufferVideo = (uchar *)malloc(m_bufferSizeVido + 1000);// 这里多分配1000个字节就基本不会出现拷贝超出的情况了，反正不缺这点内存
    m_end = false;

    return true;
}


/**
 * @brief    读取并返回视频图像
 * @return
 */
///
/// \brief   读取帧
/// \return  成功返回帧，失败返回null
///
bool FFmpegDecode::readFrame(uchar * pYUVData, int * nWidth, int * nHeight)
{
    // 如果没有打开则返回
    if(!m_formatContext)
    {
        return false;
    }

    // 读取下一帧数据
    int readRet = av_read_frame(m_formatContext, m_packet);
    if(readRet < 0)
    {
        avcodec_send_packet(m_codecContext, m_packet); // 读取完成后向解码器中传如空AVPacket，否则无法读取出最后几帧
    }
    else
    {
        if(m_packet->stream_index == m_videoIndex)     // 如果是图像数据则进行解码
        {
            // 计算当前帧时间（毫秒）
//            m_pts = av_rescale_q(m_packet->dts, m_formatContext->streams[m_videoIndex]->time_base, AV_TIME_BASE_Q);
//            m_dts = av_rescale_q(m_packet->pts, m_formatContext->streams[m_videoIndex]->time_base, AV_TIME_BASE_Q);
            m_pts = qRound64(m_packet->pts * (1000 * rationalToDouble(&m_formatContext->streams[m_videoIndex]->time_base)));
            m_dts = qRound64(m_packet->dts * (1000 * rationalToDouble(&m_formatContext->streams[m_videoIndex]->time_base)));
            // 将读取到的原始数据包传入解码器
            int ret = avcodec_send_packet(m_codecContext, m_packet);
            if(ret < 0)
            {
                qWarning() << "发送原始数据包到解码器失败";
            }
        }
    }
    av_packet_unref(m_packet);// 释放数据包，引用计数-1，为0时释放空间
    av_frame_unref(m_frame);
    int ret = avcodec_receive_frame(m_codecContext, m_frame);
    if(ret < 0)
    {
        av_frame_unref(m_frame);
        if(readRet < 0)
        {
            m_end = true;     // 当无法读取到AVPacket并且解码器中也没有数据时表示读取完成
        }
        return false;
    }

    *nHeight = m_frame->height;
    *nWidth = m_frame->width;
    int nVideoHeight = m_frame->height;
    int nVideoWidth = m_frame->width;
    int bytes =0;
    for(int i=0;i<nVideoHeight;i++){
        memcpy(pYUVData+bytes,m_frame->data[0]+m_frame->linesize[0]*i,nVideoWidth);
        bytes+=nVideoWidth;
    }

    int u=nVideoHeight>>1;
    for(int i=0;i<u;i++){
        memcpy(pYUVData+bytes,m_frame->data[1]+m_frame->linesize[1]*i,nVideoWidth/2);
        bytes+=nVideoWidth/2;
    }

    for(int i=0;i<u;i++){
        memcpy(pYUVData+bytes,m_frame->data[2]+m_frame->linesize[2]*i,nVideoWidth/2);
        bytes+=nVideoWidth/2;
    }

    av_frame_unref(m_frame);

    return true;
}

///
/// \brief 关闭视频播放并释放内存
///
void FFmpegDecode::closeMedia()
{
    clearObject();
    freeObject();

    m_totalTime     = 0;
    m_videoIndex    = 0;
    m_totalFrames   = 0;
    m_pts           = 0;
    m_frameRate     = 0;
    m_size          = QSize(0, 0);
}

///
/// \brief 视频是否读取完成
/// \return
///
bool FFmpegDecode::isEnd()
{
    return m_end;
}

///
/// \brief 获取当前视频帧图像播放时间
/// \return
///
const qint64 &FFmpegDecode::pts()
{
    return m_pts;
}

///
/// \brief 获取当前帧解码时间
/// \return
///
const qint64 &FFmpegDecode::dts()
{
    return m_dts;
}

const int& FFmpegDecode::getFrameBufferSize()
{
    return m_bufferSizeVido;
}

const qint64& FFmpegDecode::getTotalFrames()
{
    return m_totalFrames;
}

const QSize& FFmpegDecode::getVideoSize()
{
    return m_size;
}

///
/// \brief 将AVRational转换为double，用于计算帧率
/// \param rational
/// \return
///
qreal FFmpegDecode::rationalToDouble(AVRational* rational)
{
    qreal frameRate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
    return frameRate;
}

///
/// \brief 清空读取缓冲
///
void FFmpegDecode::clearObject()
{
    if(m_formatContext && m_formatContext->pb)
    {
        avio_flush(m_formatContext->pb);
    }
    if(m_formatContext)
    {
        avformat_flush(m_formatContext);// 清理读取缓冲
    }
}

///
/// \brief 释放
///
void FFmpegDecode::freeObject()
{
    if(m_swsContext)
    {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    if(m_codecContext)
    {
        avcodec_free_context(&m_codecContext);
    }
    if(m_formatContext)
    {
        avformat_close_input(&m_formatContext);
    }
    if(m_packet)
    {
        av_packet_free(&m_packet);
    }
    if(m_frame)
    {
        av_frame_unref(m_frame);
        av_frame_free(&m_frame);
    }
}
