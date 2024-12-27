#ifndef FFMPEGINCLUDE_H
#define FFMPEGINCLUDE_H

#include <QtWidgets>

//为了兼容C和C99标准必须加以下内容(否则编译不能通过)
#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif

//引入ffmpeg头文件
extern "C" {
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/frame.h"
#include "libavutil/display.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavutil/ffversion.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#if (LIBAVCODEC_VERSION_MAJOR > 56)
#include "libavutil/hwcontext.h"
#endif
}

#include "qobject.h"
#include "qdatetime.h"
#include "qdebug.h"

#ifdef Q_CC_MSVC
#pragma execution_character_set("utf-8")
#endif

#ifndef TIMEMS
#define TIMEMS qPrintable(QTime::currentTime().toString("HH:mm:ss zzz"))
#endif

#define MAX_AUDIO_FRAME_SIZE 10240

//通过avcode版本定义对应主版本
#if (LIBAVCODEC_VERSION_MAJOR == 56)
#define FFMPEG_VERSION_MAJOR 2
#elif (LIBAVCODEC_VERSION_MAJOR == 57)
#define FFMPEG_VERSION_MAJOR 3
#elif (LIBAVCODEC_VERSION_MAJOR == 58)
#define FFMPEG_VERSION_MAJOR 4
#elif (LIBAVCODEC_VERSION_MAJOR == 59)
#define FFMPEG_VERSION_MAJOR 5
#elif (LIBAVCODEC_VERSION_MAJOR == 60)
#define FFMPEG_VERSION_MAJOR 6
#endif

#if (FFMPEG_VERSION_MAJOR > 4)
#define AVCodecx const AVCodec
#define AVInputFormatx const AVInputFormat
#define AVOutputFormatx const AVOutputFormat
#else
#define AVCodecx AVCodec
#define AVInputFormatx AVInputFormat
#define AVOutputFormatx AVOutputFormat
#endif

#if (FFMPEG_VERSION_MAJOR < 3)
enum AVHWDeviceType {
    AV_HWDEVICE_TYPE_VDPAU,
    AV_HWDEVICE_TYPE_CUDA,
    AV_HWDEVICE_TYPE_VAAPI,
    AV_HWDEVICE_TYPE_DXVA2,
    AV_HWDEVICE_TYPE_QSV,
    AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
    AV_HWDEVICE_TYPE_NONE,
    AV_HWDEVICE_TYPE_D3D11VA,
    AV_HWDEVICE_TYPE_DRM,
};
#endif

//视频数据结构体
struct STRUCT_VIDEO_PACKET
{
    qint64 pts;
    qint64 dts;
    int width;
    int height;
    QByteArray pFrameData;
    qint64 nFrameDataLen;

    STRUCT_VIDEO_PACKET()
    {
        pts             = 0;
        dts             = 0;
        width           = 0;
        height          = 0;
        nFrameDataLen   = 0;
        pFrameData      = nullptr;
    }
};

//非阻塞延时
void ffsleepMsec(int msec);

typedef std::function<void(QByteArray)> DefPushbackAudioDataCallBack;          //插入音频数据回调
typedef std::function<void(STRUCT_VIDEO_PACKET)> DefPushbackVideoDataCallBack; //插入视频数据回调

#endif // FFMPEGINCLUDE_H
