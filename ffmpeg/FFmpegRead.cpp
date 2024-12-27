#include "ffmpeg/FFmpegRead.h"

FFmpegRead::FFmpegRead(QObject *parent) : QThread(parent)
{
}

FFmpegRead::~FFmpegRead()
{
    freeObject();
}

void FFmpegRead::run(){
    bool bIsError = true;
    if(m_inputFormatContext == nullptr)
    {
        qWarning("未打开视频");
        freeObject();
        emit readError();
        return;
    }
    if(m_videoCodecCtx == nullptr)
    {
        qWarning("未打开解码器");
        freeObject();
        emit readError();
        return;
    }

    m_frameCount = 0;
    m_startTime = av_gettime();
    while(!this->isInterruptionRequested()&&!isNeedEnd)
    {
        // 读取下一帧数据
        int readRet = av_read_frame(m_inputFormatContext, m_packet);
        if(readRet < 0)
        {
            av_packet_unref(m_packet);
            av_init_packet(m_packet);
            av_freep(m_packet);
            bIsError = false;
            break;
        }
        else
        {
            if(m_packet->stream_index == m_videoIndex)
            {
                m_dts = qRound64(m_packet->dts * (1000 * rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->time_base)));
                //视频解码
                if(decodeFrame() < 0)
                {
                    av_packet_unref(m_packet);
                    av_init_packet(m_packet);
                    av_freep(m_packet);
                    bIsError = true;
                    break;
                }
            }
            else if(m_packet->stream_index == m_audioIndex)
            {
                int ret = -1;
                emit readIndex(1,QPixmap(),m_packet,ret);

                //qDebug()<<"read ret:"<<ret;
//                if(ret < 0){
//                    qWarning("写入音频帧失败");
//                    av_packet_unref(m_packet);
//                    av_init_packet(m_packet);
//                    av_freep(m_packet);
//                    bIsError = true;
//                    break;
//                }
            }

            //释放当前包
            av_packet_unref(m_packet);
            av_init_packet(m_packet);
            av_freep(m_packet);
            //ffsleepMsec(1);
        }
    }
    freeObject();
    if(bIsError)
    {
        emit readError();
        return;
    }

    emit readEnd();
}

int FFmpegRead::openFile(const string filePath){
    freeObject();
    this->filePath = filePath;
    if(filePath.empty()){
        return -1;
    }

    //获取上下文对象m_inputFormatContext
    int nRet = avformat_open_input(&m_inputFormatContext,        // 返回解封装上下文
                                       filePath.c_str(),                    // 打开视频地址
                                       nullptr,                      // 如果非null，此参数强制使用特定的输入格式。自动选择解封装器（文件格式）
                                       nullptr);                     // 参数设置
    //读取流
    nRet = avformat_find_stream_info(m_inputFormatContext, nullptr);
    if(nRet < 0)
    {
        return -1;
    }
    // 通过AVMediaType枚举查询视频流ID（也可以通过遍历查找），最后一个参数无用
    m_videoIndex = av_find_best_stream(m_inputFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(m_videoIndex < 0)
    {
        qWarning("查找视频流失败");
        return -1;
    }
    //获取音频流ID
    m_audioIndex = av_find_best_stream(m_inputFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    //高和宽
    m_size.setWidth(m_inputFormatContext->streams[m_videoIndex]->codecpar->width);
    m_size.setHeight(m_inputFormatContext->streams[m_videoIndex]->codecpar->height);
    //频率
    m_fps = rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->avg_frame_rate);
    m_timeBase = m_inputFormatContext->streams[m_videoIndex]->time_base;
    AVStream* videoStream = m_inputFormatContext->streams[m_videoIndex];  // 通过查询到的索引获取视频流
    m_totalFrames = videoStream->nb_frames;
     //获取解码器
     AVCodecParameters* videocodec_params = m_inputFormatContext->streams[m_videoIndex]->codecpar;
     AVCodec* videoCodec = avcodec_find_decoder(videocodec_params->codec_id);
     if (!videoCodec)
     {
         qWarning("查找视频解码器失败");
         return -1;
     }
     qDebug() << QString("分辨率：[w:%1,h:%2] 帧率：%3  总帧数：%4  解码器：%5")
                        .arg(m_size.width()).arg(m_size.height()).arg(m_fps).arg(videoStream->nb_frames).arg(videoCodec->name);
     qDebug() << "bit_rate:" << videoStream->codecpar->bit_rate;

     //视频解码器上下文
     m_videoCodecCtx = avcodec_alloc_context3(videoCodec);
     if(!m_videoCodecCtx)
     {
         qWarning("创建视频解码器上下文失败");
         return -1;
     }
     //给上解码器上下文赋值
     nRet = avcodec_parameters_to_context(m_videoCodecCtx, videocodec_params);
     if (nRet < 0)
     {
         qWarning("视频解码器上下文赋值失败");
         return -1;
     }
     av_opt_set(m_videoCodecCtx->priv_data, "tune", "zerolatency", 0);
     //打开解码器
     nRet = avcodec_open2(m_videoCodecCtx, nullptr, nullptr);
     if(nRet < 0)
     {
         qWarning("初始化视频解码器上下文失败");
         return -1;
     }

     if(m_audioIndex>=0){
         ////////音频解码器////////
         AVCodecParameters* audiocodec_params = m_inputFormatContext->streams[m_audioIndex]->codecpar;
         const AVCodec * audioCodec = avcodec_find_decoder(audiocodec_params->codec_id);
         if (!audioCodec)
         {
             qWarning("查找音频解码器失败");
             return -1;
         }
         //创建音频解码器上下文
         m_audioCodecCtx = avcodec_alloc_context3(audioCodec);
         if(!m_audioCodecCtx)
         {
             qWarning("创建音频解码器上下文失败");
             return -1;
         }
         nRet = avcodec_parameters_to_context(m_audioCodecCtx, audiocodec_params);
         if(nRet < 0)
         {
             qWarning("音频解码器上下文赋值失败");
             return -1;
         }
         //打开音频解码器
         nRet = avcodec_open2(m_audioCodecCtx, nullptr, nullptr);
         if (nRet < 0)
         {
             qWarning("初始化音频解码器上下文失败");
             return -1;
         }
     }

     // 分配AVPacket并将其字段设置为默认值。
     m_packet = av_packet_alloc();
     if(!m_packet){
         qWarning() << "[初始化]分配视频包失败";
         freeObject();
         return false;
     }
     //初始化数据包
     av_init_packet(m_packet);

     return 0;
}

void FFmpegRead::stopRead()
{
    this->requestInterruption();
}

int FFmpegRead::decodeFrame()
{
    AVFrame* pVideoframe = av_frame_alloc();
    if(pVideoframe == nullptr)
    {
        qWarning("分配视频帧空间失败");
        return -1;
    }
    int nRet = 0;
    // 将读取到的原始数据包传入解码器
    nRet = avcodec_send_packet(m_videoCodecCtx, m_packet);
    if(nRet < 0)
    {
        qWarning("发送视频帧到解码器失败");
        av_frame_free(&pVideoframe);
        return -1;
    }

    while(nRet >= 0 && !isNeedEnd)
    {

        nRet = avcodec_receive_frame(m_videoCodecCtx, pVideoframe);
        if (nRet == AVERROR_EOF)
        {
            av_frame_unref(pVideoframe);
            av_frame_free(&pVideoframe);
            return 0;
        }
        else if(nRet == AVERROR(EAGAIN))
        {
            av_frame_unref(pVideoframe);
            av_frame_free(&pVideoframe);
            waitDts();
            return 0;
        }
        else if (nRet < 0)
        {
            qWarning("视频帧解码失败");
            av_frame_unref(pVideoframe);
            av_frame_free(&pVideoframe);
            return -1;
        }
        pVideoframe->pts = pVideoframe->best_effort_timestamp;

        QPixmap pixmapFrame = avFrameToQPixmap(pVideoframe);

        int ret = -1;
        emit readIndex(0,pixmapFrame,nullptr,ret);
        m_frameCount++;

    }
//    av_frame_unref(pVideoframe);
//    av_frame_free(&pVideoframe);
    return 0;
}

void FFmpegRead::freeObject()
{
    isNeedEnd = false;
    if(m_inputFormatContext && m_inputFormatContext->pb)
    {
        avio_flush(m_inputFormatContext->pb);
    }
    if(m_inputFormatContext)
    {
        avformat_flush(m_inputFormatContext);// 清理读取缓冲
    }
    if(m_videoCodecCtx)
    {
        avcodec_free_context(&m_videoCodecCtx);
        m_videoCodecCtx = nullptr;
    }
    if(m_audioCodecCtx)
    {
        avcodec_free_context(&m_audioCodecCtx);
        m_audioCodecCtx = nullptr;
    }
    if(m_inputFormatContext)
    {
        avformat_close_input(&m_inputFormatContext);
        avformat_free_context(m_inputFormatContext);
        m_inputFormatContext = nullptr;
    }

    if(m_packet)
    {
        av_packet_unref(m_packet);
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }
}

qreal FFmpegRead::rationalToDouble(AVRational* rational)
{
    qreal frameRate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
    return frameRate;
}

void FFmpegRead::waitDts()
{
    m_dts = qRound64(m_packet->dts * (1000 * rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->time_base)));
    qint64 currentTime = (av_gettime() - m_startTime) / 1000;
    qint64 nDelay = m_dts - currentTime;
    if (nDelay > 0)
    {
        ffsleepMsec(nDelay);
    }
    return;
}

QPixmap FFmpegRead::avFrameToQPixmap(AVFrame *frame) {
    // 假设 AVFrame 是 YUV420P 格式
    int width = frame->width;
    int height = frame->height;

    // 创建一个 QImage 用于存储 RGB 数据
    QImage image(width, height, QImage::Format_RGB888);

    // 创建一个 SwsContext 进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P,
                                                 width, height, AV_PIX_FMT_RGB24,
                                                 SWS_BICUBIC, nullptr, nullptr, nullptr);

    // 使用 sws_scale 进行格式转换
    uint8_t *rgbData = image.bits();  // 获取 QImage 数据指针
    int line = image.bytesPerLine();
    sws_scale(sws_ctx, frame->data, frame->linesize, 0, height, &rgbData, &line);

    // 释放 SwsContext
    sws_freeContext(sws_ctx);

    // 转换为 QPixmap
    QPixmap pixmap = QPixmap::fromImage(image);

    return pixmap;
}

