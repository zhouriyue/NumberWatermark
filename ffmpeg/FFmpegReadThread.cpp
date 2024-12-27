#include "FFmpegReadThread.h"

///
/// \brief 非阻塞延时
/// \param msec 延时毫秒
///
void sleepMsec(int msec)
{
    if(msec <= 0) return;
    QEventLoop loop;		//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();			//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

FFmpegReadThread::FFmpegReadThread(QObject *parent) : QThread(parent)
{
    m_ffmpegDecode = new FFmpegDecode();
}

FFmpegReadThread::~FFmpegReadThread()
{
    if(m_ffmpegDecode)
    {
        delete m_ffmpegDecode;
    }
}

///
/// \brief 传入播放的视频地址并开启线程
/// \param url
///
bool FFmpegReadThread::openMedia(const QString & strUrl)
{
    m_strUrl = strUrl;
    bool ret = m_ffmpegDecode->openMedia(m_strUrl);// 打开网络流时会比较慢，如果放到Ui线程会卡
    if(!ret)
    {
        qWarning() << "打开失败！";
        return false;
    }
    m_nTotalFrames = m_ffmpegDecode->getTotalFrames();
    m_size = m_ffmpegDecode->getVideoSize();
    return true;
}

///
/// \brief 关闭读媒体流
///
void FFmpegReadThread::closeMedia()
{
    this->requestInterruption();
}

///
/// \brief 获取当前播放的地址
/// \return
///
const QString &FFmpegReadThread::url()
{
    return m_strUrl;
}

const qint64& FFmpegReadThread::getTotalFrames()
{
    return m_nTotalFrames;
}

const QSize& FFmpegReadThread::getVideoSize()
{
    return m_size;
}

void FFmpegReadThread::run()
{
    // 循环读取视频图像
    int nFrameBufferSize = m_ffmpegDecode->getFrameBufferSize();
    if(nFrameBufferSize < 1)
    {
        qWarning() << "获取帧大小失败";
        return;
    }
    qint64 startTime = av_gettime();
    while (!this->isInterruptionRequested())
    {
        uchar * pYUVData =  (uchar *)malloc(nFrameBufferSize + 1000);
        int nWidth = 0;
        int nHeight = 0;
        if(m_ffmpegDecode->readFrame(pYUVData, &nWidth, &nHeight))
        {
            emit repaint(pYUVData, nWidth, nHeight);//发送YUV数据到显示界面，并由显示界面释放
            qint64 currentTime = (av_gettime() - startTime) / 1000;//av_gettime获取到的时间是微妙，延时函数单位时毫秒
            qint64 nDelay = m_ffmpegDecode->dts() - currentTime;
            if (nDelay > 0)
            {
                sleepMsec(nDelay);
            }
        }
        else
        {
            free(pYUVData);
            // 当前读取到无效图像时判断是否读取完成
            if(m_ffmpegDecode->isEnd())
            {
                break;
            }
            sleepMsec(1);
        }
    }
    qDebug() << "Stop read";

    m_ffmpegDecode->closeMedia();
    qDebug() << "read end";
    emit readEnd();
}
