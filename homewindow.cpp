#include "homewindow.h"
#include "ui_homewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "widget/titlebar/homeTitleBar/hometitlebar.h"
#include "utils/ParseUtil.h"
#include "utils/DecryptUtil.h"

HomeWindow::HomeWindow(QString username,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeWindow)
{
    this->username = username;
    ui->setupUi(this);

    // 禁用默认标题栏
   setWindowFlags(Qt::FramelessWindowHint);
   // 创建自定义标题栏并设置为窗口的标题栏
   HomeTitleBar *customTitleBar = new HomeTitleBar(username,this);
   connect(customTitleBar, &HomeTitleBar::avatorClickCallback, this, &HomeWindow::avatorClickCallback);
   setMenuWidget(customTitleBar);  // 将自定义标题栏作为菜单栏

   setStyleSheet("QMainWindow { "
                                "background-image: url(:/qss/lightblue/bg.png); "
                                "}");

   ui->videoWatermarkBtn->setObjectName("0");
   ui->videoWatermarkBtn->setCheckable(true);  // 确保按钮支持选中状态
   ui->videoWatermarkBtn->setChecked(true);
   ui->videoWatermarkBtn->setStyleSheet("QPushButton {"
                                        "border-radius: 0px;"   // 设置为 0，取消圆角
                                        "padding: 5px;"
                                        "font-size: 16px;"
                                        "}"
                                        "QPushButton:checked {"
                                        "  background-color: #84C1FF;"  // 选中时的颜色
                                        "  border-left: 3px solid #5A92D1;"
                                        "  color: white;"  // 文字颜色变化
                                        "}");
   ui->imgWatermarkBtn->setCheckable(true);  // 确保按钮支持选中状态
   ui->imgWatermarkBtn->setObjectName("1");
   ui->imgWatermarkBtn->setStyleSheet("QPushButton {"
                                      "border-radius: 0px;"   // 设置为 0，取消圆角
                                      "padding: 5px;"
                                      "font-size: 16px;"
                                      "}"
                                      "QPushButton:checked {"
                                      "  background-color: #84C1FF;"  // 选中时的颜色
                                      "  border-left: 3px solid #5A92D1;"
                                      "  color: white;"  // 文字颜色变化
                                      "}");
   // 将按钮连接到同一个槽函数
   connect(ui->videoWatermarkBtn, &QPushButton::clicked, this, &HomeWindow::on_navicationBtn_clicked);
   connect(ui->imgWatermarkBtn, &QPushButton::clicked, this, &HomeWindow::on_navicationBtn_clicked);

   decryptData = new DecryptData();

   m_insertWatermark = new FFmpegInsertWatermark();
   connect(m_insertWatermark, &FFmpegInsertWatermark::signalEnd, this, &HomeWindow::on_InsertStop);
   connect(m_insertWatermark, &FFmpegInsertWatermark::signalError, this, &HomeWindow::on_InsertError);
   connect(m_insertWatermark, &FFmpegInsertWatermark::progress, this, &HomeWindow::progress);

   m_insertWatermark->dct = new Dct();

   ui->outputPathEdit->setText(createOutputPath());
}

void HomeWindow::mousePressEvent(QMouseEvent *event)
{
    // 如果点击的是图片区域，则开始拖动
    if(m_pImageArea == nullptr) return;
    QPoint globalPos = m_pImageArea->mapToGlobal(QPoint(0, 0));

    int x = event->globalX();
    int y = event->globalY();

    int parentX = globalPos.x();
    int parentY = globalPos.y();

    if (x >= parentX && x <= parentX+m_pImageArea->width()
            && y>= parentY && y <= parentY+m_pImageArea->height()) {
        dragging = true;
        lastPos = event->pos() - m_pImageArea->pos();  // 记录偏移量
    }
}

void HomeWindow::mouseMoveEvent(QMouseEvent *event)
{
    QWidget *target = nullptr;
    if(selectIndex == 0){
        target = m_pFFmpegPlayWidget;
    } else {
        target = m_sourceImageArea;
    }
    if(m_pImageArea == nullptr || target == nullptr) return;
    if (dragging) {
        // 计算新的位置并移动图片
        QPoint newPos = event->pos() - lastPos;

        QRect parentRect = target->rect();
        QRect labelRect = m_pImageArea->rect();

        int newX = newPos.x();
        int newY = newPos.y();


        // 计算图片能移动的最大范围，防止超出父控件
       newX = qBound(target->x()+parentRect.left(), newX, target->x()+parentRect.right() - labelRect.width());
       newY = qBound(target->y()+parentRect.top(), newY, target->y()+parentRect.bottom() - labelRect.height());

       m_pImageArea->move(newX, newY);

    }
}

void HomeWindow::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;
}

//析构函数
HomeWindow::~HomeWindow()
{
    delete ui;
    delete decryptData;
//    if(reader)
//    {
//        if(reader->isRunning())
//        {
//            reader->stopRead();
//            reader->wait();
//        }
//        delete reader;
//        reader = nullptr;
//    }
    if(m_pImageArea)
    {
        delete m_pImageArea;
        m_pImageArea = nullptr;
    }
    if(m_pFFmpegPlayWidget)
    {
        delete m_pFFmpegPlayWidget;
        m_pFFmpegPlayWidget = nullptr;
    }
    if(m_insertWatermark)
    {
        if(m_insertWatermark->isRunning())
        {
            m_insertWatermark->stopInsert();
            m_insertWatermark->wait();
        }
        delete m_insertWatermark;
        m_insertWatermark = nullptr;
    }
}

//点击头像的回调（槽函数）
void HomeWindow::avatorClickCallback(){
    LoginWindow *login = new LoginWindow();
    login->show();
    this->close();
}

//添加文件
void HomeWindow::on_addFileBtn_clicked()
{
    if(selectIndex == 0){
         inputFilePath = QFileDialog::getOpenFileName(this, tr("打开视频"), "", tr("视频文件 (*.mp4 *.avi *.mkv)"));
         if(inputFilePath.isEmpty()) return;
         if(m_pFFmpegPlayWidget)
         {
             delete m_pFFmpegPlayWidget;
             m_pFFmpegPlayWidget = nullptr;
         }
//         if(reader->isRunning())
//         {
//             reader->requestInterruption();
//             while (!reader->isFinished()) {}
//         }
         QFileInfo fileInfo(inputFilePath);
         QString baseName = fileInfo.baseName();  // 文件名（不带扩展名）
         QString extension = fileInfo.suffix();  // 文件扩展名
         // 获取当前时间，格式化为 "yyyy-MM-dd_hh-mm-ss"
         QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
         // 生成新的文件名
         QString newFileName = fileInfo.absolutePath()+"/"+baseName+"_watermark_" + timestamp + "." + extension;
         ui->outputPathEdit->setText(newFileName);

         ui->progressLabel->setText("");
         m_pFFmpegPlayWidget = new FFmpegPlayWidget(this);
         if(!m_pFFmpegPlayWidget->onOpenMedia(inputFilePath, nTotalFrames, videoSize))
         {
             QMessageBox::warning(this,tr("加载视频失败"),tr("无法加载视频文件"));
             ui->addWatermarkBtn->setEnabled(false);
             return;
         }
         double rate = 350.0/videoSize.height();
         finalWidth = videoSize.width() * rate;
         finalHeight = 350;
         int newX = (1024-finalWidth)/2;
         m_pFFmpegPlayWidget->setGeometry(newX, 90, finalWidth, finalHeight);
         m_pFFmpegPlayWidget->show();
         ui->addWatermarkBtn->setEnabled(true);

         if(m_pImageArea)
         {
             delete m_pImageArea;
             m_pImageArea = nullptr;
         }
//         ui->addWatermarkBtn->setDisabled(false);
//         reader->openFile(inputVideoPath.toStdString());
//         reader->start();

    } else {
        inputFilePath = QFileDialog::getOpenFileName(this,tr("打开图片"),"",tr("图片文件 (*.png *.jpg *.bmp);;所有文件 (*.*)"));

        if(!inputFilePath.isEmpty()){
            QImage image(inputFilePath);

            if(image.isNull()){
                QMessageBox::warning(this,tr("加载图片失败"),tr("无法加载图片文件！"));
            } else {
                //ui->showImgLabel->setPixmap(pixmap);
                //ui->showImgLabel->setScaledContents(true);
                ui->addWatermarkBtn->setDisabled(false);

                if(m_sourceImageArea)
                {
                    delete m_sourceImageArea;
                    m_sourceImageArea = nullptr;
                }
                m_sourceImageArea = new ImageArea(this);

                double rate = 350.0/image.height();
                            finalWidth = image.width() * rate;
                            finalHeight = 350;
                            int newX = (1024-finalWidth)/2;
                m_sourceImageArea->setGeometry(newX,90,finalWidth, finalHeight);
                m_sourceImageArea->show();
                m_sourceImageArea->setImage(image);
                m_sourceQImage = image;

                QFileInfo fileInfo(inputFilePath);
                QString baseName = fileInfo.baseName();  // 文件名（不带扩展名）
                QString extension = fileInfo.suffix();  // 文件扩展名
                // 获取当前时间，格式化为 "yyyy-MM-dd_hh-mm-ss"
                QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
                // 生成新的文件名
                QString newFileName = fileInfo.absolutePath()+"/"+baseName+"_watermark_" + timestamp + "." + extension;
                ui->outputPathEdit->setText(newFileName);

                if(m_pImageArea)
                {
                    delete m_pImageArea;
                    m_pImageArea = nullptr;
                }
            }
        }
    }

}

//添加水印
void HomeWindow::on_addWatermarkBtn_clicked()
{
    QString selectImgName = QFileDialog::getOpenFileName(this,tr("导入水印"),"",tr("水印包 (*.pem)"));
    if(!selectImgName.isEmpty()){
        ParseUtil *parseUtil = new ParseUtil();
        parseUtil->parseDecryptData(selectImgName,decryptData);

        DecryptUtil *decryptUtil = new DecryptUtil();
        QImage img;
        if(decryptUtil->decryptQImage(decryptData,&img)>=0){

            QPixmap pixmap = QPixmap::fromImage(img);
            Mat mat = QPixmapToMat(pixmap);
            cv::Mat grayImage;
            // 将彩色图像转换为灰度图像
            cv::cvtColor(mat, grayImage, cv::COLOR_BGR2GRAY);
            m_insertWatermark->dct->init(grayImage,50);

            if(m_pImageArea)
            {
                delete m_pImageArea;
                m_pImageArea = nullptr;
            }
            m_pImageArea = new ImageArea(this);

            float pX = 0;
            float pY = 0;

            if(selectIndex == 0){
                pX = m_pFFmpegPlayWidget->x() + m_pFFmpegPlayWidget->width() / 2 - img.width() / 2;
                pY = m_pFFmpegPlayWidget->y() + m_pFFmpegPlayWidget->height() / 2 - img.height() / 2;
            } else {
                pX = m_sourceImageArea->x() + m_sourceImageArea->width() / 2 - img.width() / 2;
                pY = m_sourceImageArea->y() + m_sourceImageArea->height() / 2 - img.height() / 2;
            }

            m_pImageArea->setGeometry(pX,pY,img.width(), img.height());
            m_pImageArea->show();
            m_pImageArea->setImage(img);
        }
    }
}

//选择导出目录
void HomeWindow::on_outputPathBtn_clicked()
{
    QString selectDir = QFileDialog::getExistingDirectory(this,tr("选择目录"),QString(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!selectDir.isEmpty()){
        ui->outputPathEdit->setText(selectDir);
    }
}

void HomeWindow::on_navicationBtn_clicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    selectIndex = button->objectName().toInt();
    qDebug()<<"index:"<<selectIndex;
    if(selectIndex == 0){
        if(m_sourceImageArea)
        {
            delete m_sourceImageArea;
            m_sourceImageArea = nullptr;
        }
        ui->videoWatermarkBtn->setChecked(true);
        ui->imgWatermarkBtn->setChecked(false);
    } else {
        if(m_pFFmpegPlayWidget)
        {
            delete m_pFFmpegPlayWidget;
            m_pFFmpegPlayWidget = nullptr;
        }
        ui->videoWatermarkBtn->setChecked(false);
        ui->imgWatermarkBtn->setChecked(true);
    }
    if(m_pImageArea)
    {
        delete m_pImageArea;
        m_pImageArea = nullptr;
    }
}

void HomeWindow::on_startOutputBtn_clicked()
{
    if(selectIndex == 0){
        if(!isStartOutput){
            if(m_pImageArea == nullptr){
                // 创建信息提示框
                QMessageBox::information(this, "Information", "请导入水印！");
                return;
            }
            if(m_insertWatermark->isRunning())
            {
                m_insertWatermark->stopInsert();
                m_insertWatermark->wait();
            }
            QString strInFile = inputFilePath;
            QFileInfo fileInfo(strInFile);
            QString strSuffix = fileInfo.suffix();
            if(!fileInfo.exists())
            {
                qWarning("文件[%s]不存在", strInFile.toStdString().c_str());
                return;
            }
            if(strSuffix == "")
            {
                qWarning("文件[%s]格式不支持", strInFile.toStdString().c_str());
                return;
            }
            QString strOutFile = strInFile + "." + strSuffix;
            QFile::remove(strOutFile);

            if(!m_insertWatermark->openInputFile(strInFile.toStdString().c_str()))
            {
                qWarning("打开输入文件失败");
                return;
            }
            if(!m_insertWatermark->createOutputFile(ui->outputPathEdit->text().toUtf8()))
            {
                qWarning("打开输出文件失败");
                return;
            }
            m_insertWatermark->start();
            isStartOutput = true;
            ui->progressLabel->setDisabled(false);
            ui->startOutputBtn->setText("停止执行");
        } else {
            if(m_insertWatermark)
            {
                if(m_insertWatermark->isRunning())
                {
                    m_insertWatermark->stopInsert();
                    m_insertWatermark->wait();
                }
            }
            ui->startOutputBtn->setText("开始执行");
            isStartOutput = false;
            ui->progressLabel->setDisabled(true);
        }
    } else {
        if(m_sourceImageArea == nullptr){
            QMessageBox::information(this, "Information", "请添加图片！");
            return;
        }
        if(m_pImageArea == nullptr){
            // 创建信息提示框
            QMessageBox::information(this, "Information", "请导入水印！");
            return;
        }
        QPixmap pixmap = QPixmap::fromImage(m_sourceQImage);
        Mat sourceMat = QPixmapToMat(pixmap);
        //Mat sourceMat = imread("I:/daniel-deiev-mVzujj_aF2o-unsplash.jpg",IMREAD_COLOR);
        Mat *out;
        //QElapsedTimer timer;
        //timer.start();
        //Mat w = imread("I:/watermark.jpg",IMREAD_GRAYSCALE);
        //m_insertWatermark->dct->init(w,70);
        m_insertWatermark->dct->insertWatermark(sourceMat,out);
        Mat *watermark;
        m_insertWatermark->dct->getWatermark(*out,watermark);
        double mse = -1;
        m_insertWatermark->dct->computeMSE(*watermark,mse);
        qDebug()<<"img mse:"<<mse;
        //qint64 elapsedTime = timer.elapsed();
        //qDebug() << "Code executed in" << elapsedTime << "milliseconds.";
        String imgOutPath = ui->outputPathEdit->text().toStdString();
        qDebug()<<"imgOut:"<<QString::fromStdString(imgOutPath);
        QPixmap result = matToQPixmap(*out);
        result.save(ui->outputPathEdit->text());
        ui->progressLabel->setText("完成");
        QMessageBox::information(this, "Information", "导出成功！");
    }

}

void HomeWindow::progress(int current){
    double total = nTotalFrames * 1.0;
    ui->progressLabel->setText(QString::number(current/total*100, 'f', 1)+"%");
}

void HomeWindow::on_InsertStop()
{
    qDebug() << "添加数字水印成功";
    ui->startOutputBtn->setText("开始执行");
    isStartOutput = false;
    ui->progressLabel->setText("完成");
    QMessageBox::information(this, "Information", "导出成功！");
    //ui->progressLabel->setDisabled(true);
}

void HomeWindow::on_InsertError()
{
    qDebug() << "添加数字水印失败";
    ui->startOutputBtn->setText("开始执行");
    isStartOutput = false;
    ui->progressLabel->setText("失败");
    //ui->progressLabel->setDisabled(true);
}

QString HomeWindow::createOutputPath(){
    // 获取系统默认下载目录
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    // 如果下载目录不存在或为空，则设置为用户的主目录
     if (downloadPath.isEmpty()) {
         downloadPath = QDir::homePath();
     }
     return downloadPath;
}
Mat HomeWindow::QPixmapToMat(const QPixmap& pixmap){
    // 将 QPixmap 转换为 QImage
    QImage image = pixmap.toImage();

    // 确保 QImage 为 RGB 格式，否则需要转换
    if (image.format() != QImage::Format_RGB888) {
        image = image.convertToFormat(QImage::Format_RGB888);
    }

    // 将 QImage 数据转换为 OpenCV Mat
    cv::Mat mat(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());

    // 将 RGB 转换为 BGR，OpenCV 默认使用 BGR 格式
    cv::Mat matBGR;
    cv::cvtColor(mat, matBGR, cv::COLOR_RGB2BGR);

    return matBGR.clone();  // 克隆 Mat，避免共享内存
}
QPixmap HomeWindow::avFrameToQPixmap(AVFrame *frame){
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

QPixmap HomeWindow::matToQPixmap(const cv::Mat& mat)
{
    // 确保输入的mat是一个有效的图像
    if (mat.empty()) {
        return QPixmap();
    }

    // 如果是灰度图像
    if (mat.channels() == 1) {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return QPixmap::fromImage(image);
    }
    // 如果是RGB或BGR图像
    else if (mat.channels() == 3) {
        // OpenCV的BGR需要转换为RGB
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);

        QImage image(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
        return QPixmap::fromImage(image);
    }
    // 如果是4通道的图像（RGBA）
    else if (mat.channels() == 4) {
        cv::Mat rgbaMat;
        cv::cvtColor(mat, rgbaMat, cv::COLOR_BGRA2RGBA);

        QImage image(rgbaMat.data, rgbaMat.cols, rgbaMat.rows, rgbaMat.step, QImage::Format_RGBA8888);
        return QPixmap::fromImage(image);
    }

    return QPixmap();
}



