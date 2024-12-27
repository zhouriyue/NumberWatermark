#ifndef DCT_H
#define DCT_H

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

class Dct {
private:

    float alpha;
    float blockCount = 64;
    int watermarkWidth;
    int watermarkHeight;
    Mat block;
    int wX1;//水印位置
    int wY1;
    int wX2;//水印位置
    int wY2;
public:
    Mat watermarkMat;
    int init(Mat& watermark,int p);
    int init(uchar* watermarkArr, int width, int height, float p);
    int insertWatermark(Mat& srcMat,Mat*& outMat);
    int getWatermark(Mat& srcMat,Mat*& outMat);
    Mat arrayToMat(uchar* array, int width, int height, int type);
    void thresholdBinary(Mat& watermark);
    void copyToArray(Mat matt,uchar*& out, size_t& outSize);
    void getBlock(Mat& yMat, int x, int y, int rLen, int cLen,Mat& out);
    int computeMSE(Mat mat,double& result);
    uchar* getWatermarkArr();
    int getWatermarkWidth();
    int getWatermarkHeight();
    double calculateLocalContrast(Mat& block);
    double getAdaptiveWatermarkStrength(double localContrast);
};

#endif // DCT_H
