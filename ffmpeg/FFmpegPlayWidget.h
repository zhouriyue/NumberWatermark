#ifndef FFMPEGPLAYWIDGET_H
#define FFMPEGPLAYWIDGET_H

#include "FFmpegReadThread.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLWidget>

#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1

class FFmpegPlayWidget : public QOpenGLWidget,protected QOpenGLFunctions
{
public:
    FFmpegPlayWidget(QWidget *parent = nullptr);
    ~FFmpegPlayWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void updateGLVertex(int windowWidth, int widowHeight);

public slots:
    void repaint(uchar * pYUVData, int nWidth, int nHeight);                       // 设置需要绘制的图像帧
    bool onOpenMedia(const QString& strUrl, qint64 &nTotalFrames, QSize &size);    // 打开视频
    void onCloseMedia();                                // 关闭视频
    void onReplay();                                    // 重播

private:
    QOpenGLShader*          m_pVertexShader;        // 顶点着色器
    QOpenGLShader*          m_pFragmentShader;      // 片段着色器
    QOpenGLShaderProgram*   m_pShaderProgram;       // 着色器程序

    GLfloat*                m_pVertexVertices;      // 顶点矩阵

    GLuint                  m_textureIdY;           // Y 纹理对象 id
    GLuint                  m_textureIdU;           // U 纹理对象 id
    GLuint                  m_textureIdV;           // V 纹理对象 id
    GLuint                  m_textureUniformY;      // Y 纹理位置
    GLuint                  m_textureUniformU;      // U 纹理位置
    GLuint                  m_textureUniformV;      // V 纹理位置

    int                     m_videoWidth;           // 视频宽度
    int                     m_videoHeight;          // 视频高度
    uint8_t*                m_YUVData = nullptr;    // 视频数据

    QString m_strUrl = QString();
    FFmpegReadThread* m_ffmpegReadThread = nullptr;
};

#endif // FFMPEGPLAYWIDGET_H
