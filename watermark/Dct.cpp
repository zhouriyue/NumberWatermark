#include "Dct.h"
#include "ErrCode.h"
#include "qdebug.h"

using namespace nwe;

int Dct::init(Mat& watermark, int p){
    if (watermark.empty()) {
        //转换失败
        return INIT_TRANS_GRAY_ERROR;
    }

    //二值化
    thresholdBinary(watermark);

    this->watermarkMat = watermark;
    this->alpha = p;
    this->watermarkWidth = watermark.cols;
    this->watermarkHeight = watermark.rows;

    return INIT_SUCCESS;
}

int Dct::init(uchar* watermarkArr, int width, int height, float p) {

    if (watermarkArr == nullptr) {
        //空水印内容
        return INIT_NULL_WATERMARK_CONTENT;
    }

    if (width <= 0) {
        //width错误
        return INIT_WIDTH_ERROR;
    }

    if (height <= 0) {
        //height错误
        return INIT_HEIGHT_ERROR;
    }

    //转化为灰度图片
    Mat watermark = arrayToMat(watermarkArr, width, height, CV_8UC1);

    if (watermark.empty()) {
        //转换失败
        return INIT_TRANS_GRAY_ERROR;
    }

    //二值化
    thresholdBinary(watermark);

    this->watermarkMat = watermark;
    this->alpha = p;
    this->watermarkWidth = width;
    this->watermarkHeight = height;

    return INIT_SUCCESS;
}

int Dct::insertWatermark(Mat& srcMat, Mat*& outMat) {

    //sourceMat为空
    if (srcMat.empty()) {
        return INSERT_SOURCE_TRANS_MAT_ERROR;
    }

    //分离通道
    vector<Mat> allPlanes;

    Mat ycbcr;// = Mat::zeros(srcMat.rows, srcMat.cols, CV_8UC1);
    cvtColor(srcMat, ycbcr, COLOR_RGB2YCrCb);
    split(ycbcr, allPlanes);

    Mat yMat = allPlanes[1];

    //获取每一块的宽高
    int rLen = srcMat.rows / blockCount;
    int cLen = srcMat.cols / blockCount;

    //分块大小错误
    if (rLen <= 0 || cLen <= 0) {
        return INSERT_BLOCK_SIZE;
    }

    int finalR = rLen - rLen % 2;
    int finalC = cLen - cLen % 2;
    if (finalR < 5 || finalC < 5) {
        wX1 = finalC - 2;
        wY1 = finalR - 1;
        wX2 = finalC - 1;
        wY2 = finalR - 2;
    } else {
        wX1 = 3;
        wY1 = 4;
        wX2 = 4;
        wY2 = 3;
    }
    if (block.empty() || block.cols != finalC || block.rows != finalR) {
        block = Mat::zeros(finalR, finalC, CV_32F);
    }
    block.setTo(cv::Scalar(0));  // 将整个 Mat 的所有元素重置为 0
    //分块
    for (int i = 0; i < blockCount; i++) {
        for (int j = 0; j < blockCount; j++) {


            //获取当前分块
            getBlock(yMat, i, j, rLen, cLen, block);

            //分块为空
            if (block.empty()) {
                return INSERT_BLOCK_TRANS_MAT_ERROR;
            }

            double contrast = calculateLocalContrast(block);
            // 根据对比度计算水印强度
            double adaptiveAlpha = getAdaptiveWatermarkStrength(contrast);
            //qDebug()<<"adaptiveAlpha:"<<adaptiveAlpha<<endl;

            //对分块进行dct变换，获取频域数据
            dct(block, block);

            //在低频为正嵌入水印
            if ((float)watermarkMat.at<uchar>(i, j) == 1) {
                block.at<float>(wX1, wY1) = adaptiveAlpha;
                block.at<float>(wX2, wY2) = 0;

            }

            if ((float)watermarkMat.at<uchar>(i, j) == 0) {
                block.at<float>(wX1, wY1) = 0;
                block.at<float>(wX2, wY2) = adaptiveAlpha;
            }

            //进行逆变换
            idct(block, block);
            //保存其他数据
            for (int m = 0; m < finalR; m++) {
                for (int t = 0; t < finalC; t++) {
                    yMat.at<uchar>(i * rLen + m, j * cLen + t) = saturate_cast<uchar>(
                        block.at<float>(m, t));
                }
            }

        }
    }

    Mat mergedYcbcr;
    merge(allPlanes, mergedYcbcr);
    Mat rgbMat;
    cvtColor(mergedYcbcr, rgbMat, COLOR_YCrCb2RGB);

    outMat = new Mat();
    rgbMat.copyTo(*outMat);
    // 手动销毁通道数据
//    for (size_t i = 0; i < allPlanes.size(); ++i) {
//       // 手动释放每个通道的内存
//       allPlanes[i].release();
//    }

//    yMat.release();
//    block.release();

    //结果为空
    if (outMat->empty()) {
        return INSERT_RESULT_TRANS_MAT_ERROR;
    }

    //拷贝
    return INSERT_SUCCESS;
}

int Dct::getWatermark(Mat& srcMat, Mat*& outMat) {

    if (srcMat.empty()) {
        return GET_INSERT_SOURCE_TRANS_ERROR;
    }

    //分离通道
    vector<Mat> allPlanes;
    Mat ycbcr;// = Mat::zeros(height, widtht, CV_8UC1);
    cvtColor(srcMat, ycbcr, COLOR_RGB2YCrCb);
    // 直接从 RGB 图像中提取各个通道（避免不必要的颜色空间转换）
    split(ycbcr, allPlanes);


    //获取y通道数据
    Mat yMat = allPlanes[1];

    //水印图片
    Mat watermark = Mat::zeros(64, 64, CV_8UC1);

    int rLen = srcMat.rows / blockCount;
    int cLen = srcMat.cols / blockCount;

    if (rLen <= 0 || cLen <= 0) {
        return GET_BLOCK_SIZE_ERROR;
    }

    int finalR = rLen - rLen % 2;
    int finalC = cLen - cLen % 2;
    if (finalR < 5 || finalC < 5) {
        wX1 = finalC - 2;
        wY1 = finalR - 1;
        wX2 = finalC - 1;
        wY2 = finalR - 2;
    } else {
        wX1 = 3;
        wY1 = 4;
        wX2 = 4;
        wY2 = 3;
    }
    if (block.empty() || block.cols != finalC || block.rows != finalR) {
        block = Mat::zeros(finalR, finalC, CV_32F);
    }
    block.setTo(cv::Scalar(0));  // 将整个 Mat 的所有元素重置为 0
    //#pragma omp parallel for collapse(2)  // 多线程并行计算
    for (int i = 0; i < blockCount; i++) {
        for (int j = 0; j < blockCount; j++) {

            // 获取每个块
            getBlock(yMat, i, j, rLen, cLen, block);
            if (block.empty()) {
                continue;  // 如果块为空，跳过
            }

            // 对块进行 DCT 变换
            dct(block, block);

            // 获取 DCT 系数
            double a = block.at<float>(wX1, wY1);
            double c = block.at<float>(wX2, wY2);

            // 根据 DCT 系数决定水印值
            if (a >= c) {
                watermark.at<uchar>(i, j) = 1;
            }
        }
    }

    outMat = new Mat(watermark.clone());

//    // 手动销毁通道数据
//    for (size_t i = 0; i < allPlanes.size(); ++i) {
//       // 手动释放每个通道的内存
//       allPlanes[i].release();
//    }

//    watermark.release();


    return GET_SUCCESS;
}

/*比较图片  值越小相似度越高  当值为0时说明完全一样   只要比较结果小于0.1都算是同一种图片*/
int Dct::computeMSE(Mat mat,double& result) {

    //qDebug()<<"mat.size():"<<mat.size<<",watermarkMat.size():"<<watermarkMat.size<<",mat.type()"<<mat.type()<<",watermarkMat.type()"<<watermarkMat.type();
    assert(mat.size() == watermarkMat.size() && mat.type() == watermarkMat.type());

    Mat diff;
    absdiff(mat, watermarkMat, diff);
    diff.convertTo(diff, CV_32F);  // 转换为浮点型以进行计算

    diff = diff.mul(diff);  // 平方差
    Scalar sum1 = sum(diff);
    double mse = sum1[0] / (mat.total());

    //cout << "-----------msg:" << mse << endl;

    result = mse;

    //mse 小于0.1表示同一张图片
    return MES_SUCCESS;
}

void Dct::getBlock(Mat& yMat, int x, int y, int rLen, int cLen,Mat& out) {
    //Mat mat = Mat::zeros(rLen, cLen, CV_32F);
    int finalR = rLen - rLen % 2;
    int finalC = cLen - cLen % 2;
    for (int i = 0; i < finalR; i++) {
        for (int j = 0; j < finalC; j++) {
            double temp = (float)yMat.at<uchar>(x * rLen + i, y * cLen + j);
            out.at<float>(i, j) = temp;
        }
    }
    //return mat;
}

Mat Dct::arrayToMat(uchar* array, int width, int height, int type) {

    Mat mat(height, width, type, array); // 创建 Mat 对象并关联到原始数据
    return mat;
}

//二值化
void Dct::thresholdBinary(Mat& watermark) {
    for (int i = 0; i < watermark.rows; i++) {
        for (int j = 0; j < watermark.cols; j++) {
            watermark.at<uchar>(i, j) = saturate_cast<uchar>(
                watermark.at<uchar>(i, j) == 255 ? 1 : 0);
        }
    }
}

void Dct::copyToArray(Mat mat,uchar*& out, size_t& outSize) {
    outSize = mat.total() * mat.elemSize();  // total() 返回矩阵的元素个数，elemSize() 返回每个元素的字节大小
    // 深拷贝：将 Mat 数据拷贝到 uchar* 数组
    if (out == nullptr) {
        out = new uchar[outSize];
    }
    memcpy(out, mat.data, outSize);
}

uchar* Dct::getWatermarkArr() {
    return watermarkMat.data;
}

int Dct::getWatermarkWidth() {
    return watermarkWidth;
}
int Dct::getWatermarkHeight() {
    return watermarkHeight;
}

// 计算局部对比度（使用标准差来衡量对比度）
double Dct::calculateLocalContrast(Mat& block) {
    cv::Mat mean, stddev;
    cv::meanStdDev(block, mean, stddev);  // 计算块的均值和标准差
    return stddev.at<double>(0, 0);       // 返回标准差作为对比度
}

// 根据对比度调整水印强度
double Dct::getAdaptiveWatermarkStrength(double localContrast) {
    // 对比度较低，水印强度较大
    if (localContrast < 10.0) {
        return alpha;  // 强水印
    }
    // 对比度较高，水印强度较小
    else {
        return alpha * 0.5;  // 中等强度
    }
}
