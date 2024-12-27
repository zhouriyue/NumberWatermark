#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <QDebug>
#include "loginwindow.h"
#include "entity/DecryptData.h"
//#include "ffmpeg/FFmpegRead.h"
#include "ffmpeg/FFmpegInsertWatermark.h"
#include "ffmpeg/FFmpegPlayWidget.h"
#include "ImageArea.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/opt.h"
}

using namespace cv;

namespace Ui {
class HomeWindow;
}

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeWindow(QString username,QWidget *parent = nullptr);
    ~HomeWindow();

    QString createOutputPath();
    Mat QPixmapToMat(const QPixmap& pixmap);
    QPixmap avFrameToQPixmap(AVFrame *frame);
    QPixmap matToQPixmap(const cv::Mat& mat);

public slots:
    void avatorClickCallback();
    void progress(int current);
    void on_InsertStop();
    void on_InsertError();
//    void readIndexFrame(int type,QPixmap pixmapFrame,AVPacket* audioData,int ret);
//    void readEndIndexFrame();
//    void readErrorIndexFrame();


private slots:
    void on_addFileBtn_clicked();

    void on_addWatermarkBtn_clicked();

    void on_navicationBtn_clicked();

    void on_outputPathBtn_clicked();

    void on_startOutputBtn_clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::HomeWindow *ui;

    bool dragging;                                    // 是否正在拖动
    QPoint lastPos;                                   // 鼠标按下时的偏移位置

    int selectIndex = 0;                              // 水印添加类型
    DecryptData *decryptData;                         // 解密数据
    //FFmpegRead *reader;                               // 读取首帧
    ImageArea * m_sourceImageArea = nullptr;          // 源图片组件（需要加水印的图片）
    QImage m_sourceQImage;                            // 源图片
    ImageArea * m_pImageArea = nullptr;               // 水印预览窗口
    FFmpegPlayWidget * m_pFFmpegPlayWidget = nullptr; // 视频播放窗口类
    qint64 nTotalFrames;                              // 当前视频的总帧数
    QSize videoSize;                                  // 当前视频的尺寸
    QString inputFilePath;                            // 输入文件路径
    FFmpegInsertWatermark *m_insertWatermark;         // 水印嵌入
    bool isStartOutput = false;                       // 是否开始导入
    int finalWidth;
    int finalHeight;
    QString username;                                 //用户名
};

#endif // HOMEWINDOW_H
