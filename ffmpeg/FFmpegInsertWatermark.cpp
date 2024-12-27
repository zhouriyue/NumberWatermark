#include "FFmpegInsertWatermark.h"

void bgrToYuv(const cv::Mat &bgrMat, AVFrame *frame);

FFmpegInsertWatermark::FFmpegInsertWatermark(QObject *parent) : QThread(parent)
{

}

FFmpegInsertWatermark::~FFmpegInsertWatermark()
{
    freeObject();
    delete dct;
}

bool FFmpegInsertWatermark::openInputFile(const char* pFilename)
{
    success = 0;
    fail = 0;
    freeObject();
    if(pFilename == nullptr)
    {
        freeObject();
        qWarning("输入视频文件名为空");
        return false;
    }
    else if(strlen(pFilename) == 0)
    {
        freeObject();
        qWarning("输入视频文件名为空");
        return false;
    }

    // 打开输入流并返回解封装上下文
    int nRet = avformat_open_input(&m_inputFormatContext,        // 返回解封装上下文
                                   pFilename,                    // 打开视频地址
                                   nullptr,                      // 如果非null，此参数强制使用特定的输入格式。自动选择解封装器（文件格式）
                                   nullptr);                     // 参数设置
    // 打开视频失败
    if(nRet < 0)
    {
        freeObject();
        return false;
    }
    // 查找流
    nRet = findStream();
    if(nRet != 0)
    {
        freeObject();
        return false;
    }

    // 打开解码器
    nRet = openDecoder();
    if(nRet != 0)
    {
        freeObject();
        return false;
    }

    return initObject();
}

bool FFmpegInsertWatermark::createOutputFile(const char* pFilename)
{
    if(pFilename == nullptr)
    {
        freeObject();
        qWarning("输出视频文件名为空");
        return false;
    }
    else if(strlen(pFilename) == 0)
    {
        freeObject();
        qWarning("输出视频文件名为空");
        return false;
    }

    if(initEncoderCodec() != 0)
    {
        freeObject();
        return false;
    }

    //打开编码器
    int ret = avcodec_open2(m_videoEnCodecCtx, m_videoEnCodec, nullptr);
    if (ret < 0)
    {
        freeObject();
        qWarning("打开编码器失败");
        return false;
    }

    if (avformat_alloc_output_context2(&m_outputFormatContext, nullptr, nullptr, pFilename) < 0)
    {
        freeObject();
        qWarning("创建输出格式上下文失败");
        return false;
    }

    //打开输出流
    ret = avio_open(&m_outputFormatContext->pb, pFilename, AVIO_FLAG_READ_WRITE);
    if (ret < 0)
    {
        freeObject();
        qWarning("打开输出流失败");
        return false;
    }

    //创建输出流
    for (unsigned int index = 0; index < m_inputFormatContext->nb_streams; index++)
    {
        if (index == (unsigned int)m_videoIndex)
        {
            AVStream * stream = avformat_new_stream(m_outputFormatContext, m_videoEnCodec);
            if (!stream)
            {
                freeObject();
                qWarning("创建输出视频频流失败");
                return false;
            }
            if(avcodec_parameters_from_context(stream->codecpar, m_videoEnCodecCtx) < 0)
            {
                freeObject();
                qWarning("复制视频编码器参数失败");
                return false;
            }
            stream->codecpar->codec_tag = 0;
            stream->time_base = m_inputFormatContext->streams[m_videoIndex]->time_base;
        }
        else if (index == (unsigned int)m_audioIndex)
        {
            AVStream * stream = avformat_new_stream(m_outputFormatContext, nullptr);
            if (!stream)
            {
                freeObject();
                qWarning("创建输出音频频流失败");
                return false;
            }
            stream->codecpar = m_inputFormatContext->streams[m_audioIndex]->codecpar;
            stream->codecpar->codec_tag = 0;
        }
    }

    if(writeHeader(pFilename) != 0)
    {
        freeObject();
        return false;
    }

    return true;
}

void FFmpegInsertWatermark::stopInsert()
{
    this->requestInterruption();
}

void FFmpegInsertWatermark::run()
{
    bool bIsError = true;
    if(m_inputFormatContext == nullptr || m_outputFormatContext == nullptr)
    {
        qWarning("未打开视频");
        freeObject();
        emit signalError();
        return;
    }
    if(m_videoCodecCtx == nullptr)
    {
        qWarning("未打开解码器");
        freeObject();
        emit signalError();
        return;
    }

    m_frameCount = 0;
    m_startTime = av_gettime();
    while(!this->isInterruptionRequested())
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
                if(decodeVideoFrame() < 0)
                {
                    av_packet_unref(m_packet);
                    av_init_packet(m_packet);
                    av_freep(m_packet);
                    bIsError = true;
                    break;
                }
            }
            else if(m_audioIndex >= 0 && m_packet->stream_index == m_audioIndex)
            {
                if(writePacket(m_packet, m_audioIndex) < 0)
                {
                    qWarning("写入音频帧失败");
                    av_packet_unref(m_packet);
                    av_init_packet(m_packet);
                    av_freep(m_packet);
                    bIsError = true;
                    break;
                }
//                m_packet->pts = av_rescale_q_rnd(m_packet->pts, m_inputFormatContext->streams[m_audioIndex]->time_base, m_outputFormatContext->streams[m_audioIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//                m_packet->dts = av_rescale_q_rnd(m_packet->dts, m_inputFormatContext->streams[m_audioIndex]->time_base, m_outputFormatContext->streams[m_audioIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//                m_packet->stream_index = m_audioIndex;
//                if (av_interleaved_write_frame(m_outputFormatContext, m_packet) < 0)
//                {
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

    writeFrame(nullptr);

    qDebug()<<"succes"<<success<<",fail:"<<fail;
    qDebug() << "I帧:" << nI << "B帧：" << nB << "P帧:" << nP << "其他:" << nO;

    writeTrailer();
    freeObject();
    if(bIsError)
    {
        emit signalError();
    }
    else
    {
        emit signalEnd();
    }
}

bool FFmpegInsertWatermark::initObject()
{
    // 分配AVPacket并将其字段设置为默认值。
    m_packet = av_packet_alloc();
    if(!m_packet)
    {
        qWarning() << "[初始化]分配视频包失败";
        freeObject();
        return false;
    }
    av_init_packet(m_packet);
    // 由于传递时是浅拷贝，可能显示类还没处理完成，所以如果播放完成就释放可能会崩溃；
//    if(m_bufferVideo)
//    {
//        free(m_bufferVideo);
//        m_bufferVideo = nullptr;
//    }

//    // 分配图像空间
//    int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_size.width(), m_size.height(), 4);
//    m_bufferVideo = (uchar *)malloc(size + 128);    // 这里多分配128个字节就基本不会出现拷贝超出的情况了，反正不缺这点内存
    return true;
}

void FFmpegInsertWatermark::clearObject()
{
    if(m_inputFormatContext && m_inputFormatContext->pb)
    {
        avio_flush(m_inputFormatContext->pb);
    }
    if(m_inputFormatContext)
    {
        avformat_flush(m_inputFormatContext);// 清理读取缓冲
    }
}

void FFmpegInsertWatermark::freeObject()
{
    clearObject();
    if(m_videoCodecCtx)
    {
        avcodec_free_context(&m_videoCodecCtx);
        m_videoCodecCtx = nullptr;
    }
    if(m_videoEnCodecCtx)
    {
        avcodec_free_context(&m_videoEnCodecCtx);
        m_videoEnCodecCtx = nullptr;
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
    if(m_outputFormatContext)
    {
        if(m_outputFormatContext->pb)
        {
            avio_closep(&m_outputFormatContext->pb);
        }
        m_outputFormatContext = nullptr;
    }

    if(m_packet)
    {
        av_packet_unref(m_packet);
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }
//    if(m_bufferVideo)
//    {
//        free(m_bufferVideo);
//        m_bufferVideo = nullptr;
//    }
}

int FFmpegInsertWatermark::findStream()
{
    // 读取媒体文件的数据包以获取流信息。
    int nRet = avformat_find_stream_info(m_inputFormatContext, nullptr);
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

    m_audioIndex = av_find_best_stream(m_inputFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
//    if (m_audioIndex < 0)
//    {
//        qWarning("查找音频流失败");
//        return -1;
//    }

    m_size.setWidth(m_inputFormatContext->streams[m_videoIndex]->codecpar->width);
    m_size.setHeight(m_inputFormatContext->streams[m_videoIndex]->codecpar->height);
    m_fps = rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->avg_frame_rate);
    m_timeBase = m_inputFormatContext->streams[m_videoIndex]->time_base;

    AVStream* videoStream = m_inputFormatContext->streams[m_videoIndex];  // 通过查询到的索引获取视频流
            // 获取视频图像分辨率（AVStream中的AVCodecContext在新版本中弃用，改为使用AVCodecParameters）
            m_size.setWidth(videoStream->codecpar->width);
            m_size.setHeight(videoStream->codecpar->height);
            int m_frameRate = rationalToDouble(&videoStream->avg_frame_rate);  // 视频帧率
            // 通过解码器ID获取视频解码器（新版本返回值必须使用const）
            const AVCodec* videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
            if (videoCodec == NULL)
            {
                qWarning() << TIMEMS << "查找视频解码器失败";
                freeObject();
                return false;
            }
            int m_totalFrames = videoStream->nb_frames;

            qDebug() << QString("分辨率：[w:%1,h:%2] 帧率：%3  总帧数：%4  解码器：%5")
                        .arg(m_size.width()).arg(m_size.height()).arg(m_frameRate).arg(m_totalFrames).arg(videoCodec->name);
            qDebug() << "bit_rate:" << videoStream->codecpar->bit_rate;

    return 0;
}

int FFmpegInsertWatermark::openDecoder()
{
    ////////视频解码器////////
    AVCodecParameters* videocodec_params = m_inputFormatContext->streams[m_videoIndex]->codecpar;
    AVCodec* videoCodec = avcodec_find_decoder(videocodec_params->codec_id);
    if (!videoCodec)
    {
        qWarning("查找视频解码器失败");
        return -1;
    }
    m_videoCodecCtx = avcodec_alloc_context3(videoCodec);
    if(!m_videoCodecCtx)
    {
        qWarning("创建视频解码器上下文失败");
        return -1;
    }
    int nRet = avcodec_parameters_to_context(m_videoCodecCtx, videocodec_params);
    if (nRet < 0)
    {
        qWarning("视频解码器上下文赋值失败");
        return -1;
    }
    //m_videoCodecCtx->flags2 |= AV_CODEC_FLAG2_FAST;
    //m_videoCodecCtx->thread_count = 4;

    //设置解码线程不用等待缓冲区填满
    av_opt_set(m_videoCodecCtx->priv_data, "tune", "zerolatency", 0);

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


    return 0;
}

int FFmpegInsertWatermark::initEncoderCodec()
{
    //m_videoEnCodec = avcodec_find_encoder(m_inputFormatContext->streams[m_videoIndex]->codecpar->codec_id);
    m_videoEnCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (m_videoEnCodec == nullptr)
    {
        qWarning() << "查找编码器失败";
        return  -1;
    }
    //指定编码器的参数
    m_videoEnCodecCtx = avcodec_alloc_context3(m_videoEnCodec);
    if(m_videoEnCodec == nullptr)
    {
        qWarning() << "分配编码器上下文失败";
        return  -1;
    }
    m_videoEnCodecCtx->time_base = m_inputFormatContext->streams[m_videoIndex]->time_base;
    m_videoEnCodecCtx->flags |= AV_CODEC_FLAG_QSCALE;
    m_videoEnCodecCtx->sample_fmt = m_videoCodecCtx->sample_fmt;// AV_SAMPLE_FMT_S16
    m_videoEnCodecCtx->width = m_size.width();
    m_videoEnCodecCtx->height = m_size.height();
    //m_videoEnCodecCtx->bit_rate = m_inputFormatContext->streams[m_videoIndex]->codecpar->bit_rate;
    m_videoEnCodecCtx->bit_rate = m_inputFormatContext->streams[m_videoIndex]->codecpar->bit_rate * 10;
    m_videoEnCodecCtx->pix_fmt = (AVPixelFormat)*m_videoEnCodec->pix_fmts;

    //临时
    //m_videoEnCodecCtx->framerate = m_inputFormatContext->streams[m_videoIndex]->avg_frame_rate;
    m_videoEnCodecCtx->framerate = AVRational{(int)rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->avg_frame_rate), 1};
    //m_videoEnCodecCtx->frame_size = m_inputFormatContext->streams[m_videoIndex]->codecpar->frame_size;
    //qDebug()<<"gop:"<<m_videoEnCodecCtx->framerate.num;
    //m_videoEnCodecCtx->gop_size = m_videoEnCodecCtx->framerate.num;  // GOP 大小，单位为毫秒


    //m_videoEnCodecCtx->gop_size = 1000;
    //m_videoEnCodecCtx->max_b_frames = 2;  // B 帧数量

//    if(m_inputFormatContext->streams[m_videoIndex]->codecpar->codec_id == AV_CODEC_ID_H264)
//    {
//        m_videoEnCodecCtx->profile = FF_PROFILE_H264_HIGH;
//    }
    m_videoEnCodecCtx->profile = FF_PROFILE_H264_MAIN;
    //编码器的配置;与输入流一致
        //m_videoEnCodecCtx->profile = m_inputFormatContext->streams[m_videoIndex]->codecpar->profile;
    //m_videoEnCodecCtx->level = m_inputFormatContext->streams[m_videoIndex]->codecpar->level;
    m_videoEnCodecCtx->level = 41;
    //m_videoEnCodecCtx->thread_count = 4;
//    //设置编码器0延时，否则会有接收编码结果返回-11情况（这个会影响视频帧的效果，暂屏蔽）
    av_opt_set(m_videoEnCodecCtx->priv_data, "preset", "superfast", 0);
    av_opt_set(m_videoEnCodecCtx->priv_data, "tune", "zerolatency", 0);
    //av_opt_set(m_videoEnCodecCtx->priv_data, "preset", "ultrafast", 0);
    //av_opt_set(m_videoEnCodecCtx->priv_data, "crf", "0", 0);
    //av_opt_set(m_videoEnCodecCtx->priv_data, "profile", "high444", 0);
    return 0;
}

int FFmpegInsertWatermark::decodeVideoFrame()
{
    AVFrame * pVideoframe = av_frame_alloc();
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

    while(nRet >= 0)
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

        if (pVideoframe->pict_type == AV_PICTURE_TYPE_I) {
            nI++;
        } else if (pVideoframe->pict_type == AV_PICTURE_TYPE_P) {
            nP++;
        } else if (pVideoframe->pict_type == AV_PICTURE_TYPE_B) {
            nB++;
        } else {
            nO++;
        }

        pVideoframe->pts = pVideoframe->best_effort_timestamp;
        if(isInsert){
            //pVideoframe->pict_type = AV_PICTURE_TYPE_NONE;
            //pSrcFrame加水印
            Mat* outMat = nullptr;
            nRet = insertWatermark(pVideoframe,outMat);
            if(nRet != 0)
            {
                qWarning() << "加水印失败:" << nRet;
                if(outMat != nullptr)
                {
                    delete outMat;
                }
                av_frame_unref(pVideoframe);
                av_frame_free(&pVideoframe);
                return nRet;
            }

            //拷贝
            //bgrToYuv(*outMat,pVideoframe);
            AVFrame* finalFrame = matToAvFrame(*outMat);
            finalFrame->pts = pVideoframe->best_effort_timestamp;
            finalFrame->pict_type = AV_PICTURE_TYPE_NONE;

            nRet = writeFrame(finalFrame);

            av_frame_unref(finalFrame);
            av_frame_free(&finalFrame);

            outMat->release();
            delete outMat;
            m_frameCount++;
            //qDebug() << "写入第" << m_frameCount++ << "帧视频";
        } else {
            //比较水印
            bool flag = compareWatermark(pVideoframe);
            if(flag){
                success++;
            }  else {
                fail++;
            }

            nRet = writeFrame(pVideoframe);
        }




        emit progress(m_frameCount);
        av_frame_unref(pVideoframe);
        if(nRet < 0)
        {
            av_frame_free(&pVideoframe);
            return nRet;
        }

    }

    av_frame_free(&pVideoframe);
    return 0;
}

int FFmpegInsertWatermark::writeHeader(const char* pFilename)
{
    qDebug() << "写入帧头";
    if (avio_open(&m_outputFormatContext->pb, pFilename, AVIO_FLAG_WRITE) < 0)
    {
        qWarning("打开输出文件失败");
        return -1;
    }
    if (avformat_write_header(m_outputFormatContext, nullptr) < 0)
    {
        qWarning("写入视频头失败");
        return -1;
    }
    return 0;
}

int FFmpegInsertWatermark::writeFrame(AVFrame* pFrame)
{
    AVPacket * pPacket = av_packet_alloc();
    if (pPacket == nullptr)
    {
        qWarning("写入视频帧(分配编码视频包)失败");
        return -1;
    }
    int nRet = avcodec_send_frame(m_videoEnCodecCtx, pFrame);
    if (nRet < 0)
    {
        av_packet_free(&pPacket);
        qWarning("发送视频帧到编码器失败");
        return nRet;
    }
    while(nRet >= 0)
    {
        nRet = avcodec_receive_packet(m_videoEnCodecCtx, pPacket);
        if (nRet == AVERROR_EOF)
        {
            av_packet_unref(pPacket);
            av_packet_free(&pPacket);
            return 0;
        }
        else if(nRet == AVERROR(EAGAIN))
        {
            av_packet_unref(pPacket);
            av_packet_free(&pPacket);
            waitDts();
            return 0;
        }
        else if (nRet < 0)
        {
            av_packet_unref(pPacket);
            av_packet_free(&pPacket);
            qWarning("接收频帧编码包失败");
            return -1;
        }

        waitDts();

        pPacket->pts = av_rescale_q_rnd(pPacket->pts, m_timeBase, m_outputFormatContext->streams[m_videoIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
        pPacket->dts = av_rescale_q_rnd(pPacket->dts, m_timeBase, m_outputFormatContext->streams[m_videoIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
        pPacket->stream_index = m_videoIndex;
        int nCode = av_interleaved_write_frame(m_outputFormatContext, pPacket);
        av_packet_unref(pPacket);
        if (nCode < 0)
        {
            qWarning("视频帧编码出错");
        }
        else
        {
            //qDebug() << "写入第" << m_frameCount++ << "帧视频";
        }
        if (nRet < 0)
        {
            qWarning("频帧编码失败");
            av_packet_free(&pPacket);
            return -1;
        }
    }
    av_packet_free(&pPacket);
    return 0;
}

int FFmpegInsertWatermark::writePacket(AVPacket * packet, int nIndex)
{
    packet->pts = av_rescale_q_rnd(packet->pts, m_inputFormatContext->streams[nIndex]->time_base, m_outputFormatContext->streams[nIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    packet->dts = av_rescale_q_rnd(packet->dts, m_inputFormatContext->streams[nIndex]->time_base, m_outputFormatContext->streams[nIndex]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
    packet->stream_index = nIndex;
    int nRet = av_interleaved_write_frame(m_outputFormatContext, m_packet);
    return nRet;
}

int FFmpegInsertWatermark::writeTrailer()
{
    qDebug() << "写入帧尾";
    if(m_outputFormatContext)
    {
        av_write_trailer(m_outputFormatContext);
    }
    return 0;
}

void FFmpegInsertWatermark::waitDts()
{
    if(m_packet->dts < 0) return;
    m_dts = qRound64(m_packet->dts * (1000 * rationalToDouble(&m_inputFormatContext->streams[m_videoIndex]->time_base)));
    qint64 currentTime = (av_gettime() - m_startTime) / 1000;
    qint64 nDelay = m_dts - currentTime;
    if (nDelay > 0)
    {
        ffsleepMsec(nDelay);
    }
    return;
}

qreal FFmpegInsertWatermark::rationalToDouble(AVRational* rational)
{
    qreal frameRate = (rational->den == 0) ? 0 : (qreal(rational->num) / rational->den);
    return frameRate;
}

cv::Mat avFrameToMat(AVFrame *frame) {
    if (!frame) {
        qDebug() << "AVFrame is NULL";
        return cv::Mat();
    }

    int width = frame->width;
    int height = frame->height;

    // 创建一个 SwsContext 用于格式转换
    struct SwsContext* sws_ctx = sws_getContext(width, height, (AVPixelFormat)frame->format,
                                                 width, height, AV_PIX_FMT_YUV420P,
                                                 SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
        qDebug() << "sws_getContext failed!";
        return cv::Mat();
    }

    // 分配内存存储转换后的 YUV 数据
    int y_size = width * height;
    int uv_size = y_size / 4;
    unsigned char* pFrameData = (unsigned char*)av_malloc(y_size + uv_size * 2);

    // Y, U, V 数据指针
    uint8_t* dst_data[3] = { pFrameData, pFrameData + y_size, pFrameData + y_size + uv_size };
    int dst_linesize[3] = { width, width / 2, width / 2 };  // 对应 Y, U, V 的行间距

    // 使用 sws_scale 执行格式转换
    sws_scale(sws_ctx, frame->data, frame->linesize, 0, height, dst_data, dst_linesize);
    sws_freeContext(sws_ctx);

    // 将转换后的 YUV 数据映射到 OpenCV Mat
    cv::Mat yuv_img(height + height / 2, width, CV_8UC1, pFrameData);
    cv::Mat rgb_img(height, width, CV_8UC3);

    // 将 YUV 转换为 BGR
    cv::cvtColor(yuv_img, rgb_img, cv::COLOR_YUV2BGR_I420);

    // 释放分配的内存
    av_free(pFrameData);

    return rgb_img;
}

// 将 cv::Mat 转换为 AVFrame
AVFrame* matToAvFrame(const cv::Mat& mat)
{
    AVFrame* avframe = av_frame_alloc();
    avframe->format = AV_PIX_FMT_YUV420P;
    avframe->width = mat.cols;
    avframe->height = mat.rows;
    av_frame_get_buffer(avframe, 0);
    av_frame_make_writable(avframe);
    cv::Mat yuv; // convert to yuv420p first
    cv::cvtColor(mat, yuv, cv::COLOR_BGR2YUV_I420);
    // calc frame size
    int frame_size = mat.cols * mat.rows;
    unsigned char *pdata = yuv.data;
    memcpy(avframe->data[0], pdata, frame_size);
    memcpy(avframe->data[1], pdata + frame_size, frame_size / 4);
    memcpy(avframe->data[2], pdata + frame_size * 5 / 4, frame_size / 4);
    return avframe;
}

int FFmpegInsertWatermark::insertWatermark(AVFrame *frame,Mat*& outMat){
    Mat mat = avFrameToMat(frame);
    int nRet = dct->insertWatermark(mat,outMat);
    mat.release();
    if(nRet != 0)
    {
        fail++;
        return nRet;
    }
    else
    {

        success++;
        return 0;
    }
}

void bgrToYuv(const cv::Mat &bgrMat, AVFrame *frame) {
    int width = bgrMat.cols;
    int height = bgrMat.rows;

    // Step 1: Allocate memory for AVFrame
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;  // Setting YUV420P format

    // Allocate frame buffer using av_image_alloc
    int ret = av_image_alloc(frame->data, frame->linesize, width, height, AV_PIX_FMT_YUV420P, 32);
    if (ret < 0) {
        std::cerr << "Could not allocate raw picture buffer for AVFrame." << std::endl;
        return;
    }

    // Step 2: Convert BGR to YUV420P using OpenCV
    cv::Mat yuvMat(height + height / 2, width, CV_8UC1);  // YUV420P has 1.5x the height of Y for UV

    // Convert the BGR image to YUV420P format
    cv::cvtColor(bgrMat, yuvMat, cv::COLOR_BGR2YUV_I420);

    // Step 3: Copy Y, U, V planes to AVFrame
    int ySize = width * height;
    int uvSize = width * height / 4;

    // Y plane
    memcpy(frame->data[0], yuvMat.data, ySize);
    // U plane
    memcpy(frame->data[1], yuvMat.data + ySize, uvSize);
    // V plane
    memcpy(frame->data[2], yuvMat.data + ySize + uvSize, uvSize);

    // Release yuvMat
    yuvMat.release();
}


bool FFmpegInsertWatermark::compareWatermark(AVFrame* frame)
{
    Mat mat = avFrameToMat(frame);
    Mat* outMat;
    dct->getWatermark(mat,outMat);
    double mse = 1;
    dct->computeMSE(*outMat,mse);
    qDebug()<<"mse:"<<mse<<" type:"<<frame->pict_type;
    if(mse <= 0.1)
    {
        return true;
    }
    else
    {
//        static int nFailedCount;
//        QString strFileName = "/tmp/Failed" + QString::number(nFailedCount++) + ".png";
//        cv::imwrite(strFileName.toStdString(), mat);
        qDebug()<<"提取错误 mse:"<<mse<<" type:"<<frame->pict_type;
        return false;
    }
}


