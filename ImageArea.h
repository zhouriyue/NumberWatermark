#ifndef IMAGEAREA_H
#define IMAGEAREA_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>

class ImageArea : public QOpenGLWidget,protected QOpenGLFunctions
{
public:
    ImageArea(QWidget *parent = nullptr);
    ~ImageArea();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public slots:
    void setImage(const QImage &image);
    void initTextures();
    void initShaders();

private:
    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QOpenGLShaderProgram program;
    QOpenGLTexture *texture;
    QMatrix4x4 projection;
};

#endif // IMAGEAREA_H
