#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "libavutil/opt.h"
}

using namespace cv;
using namespace std;

class ImageUtil{
public:

};

#endif // IMAGEUTIL_H
