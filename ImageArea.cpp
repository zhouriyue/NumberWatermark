#include "ImageArea.h"

ImageArea::ImageArea(QWidget *parent)  : QOpenGLWidget(parent)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);
    texture = nullptr;
}

ImageArea::~ImageArea()
{
    if(texture != nullptr)
    {
        makeCurrent();
        texture->destroy();
        delete texture;
        doneCurrent();
    }
}

void ImageArea::initializeGL()
{
    initializeOpenGLFunctions(); //初始化OPenGL功能函数
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);    //设置背景为黑色
    glEnable(GL_TEXTURE_2D);     //设置纹理2D功能可用
    initTextures();              //初始化纹理设置
    initShaders();               //初始化shaders
}

void ImageArea::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void ImageArea::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除屏幕缓存和深度缓冲
    glClearColor(1.0, 1.0, 1.0, 1.0);
    if(texture)
    {
        program.enableAttributeArray(0);
        program.enableAttributeArray(1);
        program.setAttributeArray(0, vertices.constData());
        program.setAttributeArray(1, texCoords.constData());
        program.setUniformValue("texture", 0); //将当前上下文中位置的统一变量设置为value

        if(texture->isCreated())
        {
            texture->bind();  //绑定纹理
        }

        QMatrix4x4 matrix;
        matrix.setToIdentity();
        matrix.translate(0, 0,0.0);
        matrix.rotate(0,0,0,1);
        matrix.scale(1.0);

        program.setUniformValue("transform", matrix);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);//绘制纹理
    }
}

void ImageArea::setImage(const QImage &image)
{
    if(NULL != texture)
    {
        texture->release();
        texture->destroy();
        texture->create();
        texture->setData(image); //设置纹理图像
        //设置纹理细节
        texture->setLevelofDetailBias(-1);//值越小，图像越清晰
        update();
    }
}

void ImageArea::initTextures()
{
    //QImage image(":/images/scene.jpg");
    // 加载 Avengers.jpg 图片
    //texture = new QOpenGLTexture(image);
    texture = new QOpenGLTexture(QImage());
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    //重复使用纹理坐标
    //纹理坐标(1.1, 1.2)与(0.1, 0.2)相同
    texture->setWrapMode(QOpenGLTexture::Repeat);
    //分配储存空间
    texture->allocateStorage();
}

void ImageArea::initShaders()
{
    //纹理坐标
    texCoords.append(QVector2D(0, 1)); //左上
    texCoords.append(QVector2D(1, 1)); //右上
    texCoords.append(QVector2D(0, 0)); //左下
    texCoords.append(QVector2D(1, 0)); //右下
    //顶点坐标
    vertices.append(QVector3D(-1, -1, 1));//左下
    vertices.append(QVector3D(1, -1, 1)); //右下
    vertices.append(QVector3D(-1, 1, 1)); //左上
    vertices.append(QVector3D(1, 1, 1));  //右上
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
            "attribute vec2 vertex;\n"
            "attribute vec2 texCoord;\n"
            "varying vec2 texc;\n"
            "uniform mat4 transform;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = transform*vec4(vertex, 0.0, 1.0);\n"
            "    texc = texCoord;\n"
            "}\n";
    vshader->compileSourceCode(vsrc);//编译顶点着色器代码

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
            "uniform sampler2D texture;\n"
            "varying vec2 texc;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture2D(texture,texc);\n"
            "}\n";
    fshader->compileSourceCode(fsrc); //编译纹理着色器代码

    program.addShader(vshader);//添加顶点着色器
    program.addShader(fshader);//添加纹理碎片着色器
    program.bindAttributeLocation("vertex", 0);//绑定顶点属性位置
    program.bindAttributeLocation("texCoord", 1);//绑定纹理属性位置
    // 链接着色器管道
    if (!program.link())
        close();
    // 绑定着色器管道
    if (!program.bind())
        close();
}
