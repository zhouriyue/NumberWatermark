#ifndef DECRYPTUTIL_H
#define DECRYPTUTIL_H

#include "entity/DecryptData.h"
#include "watermarkdecrypt.h"
#include <QDebug>
#include <QImage>
#include <QPixmap>

using namespace std;


class DecryptUtil{

public:
    int decryptImage(DecryptData *decryptData,QPixmap *out){
        int nRc = 0;
        string strSM4Key = "";
        nRc = LS_Get_SM4Key(
                    decryptData->identification.toStdString(),
                    "listenVision",
                    strSM4Key);
        if(nRc != 0)
        {
            qDebug()<<"sm4Key error";
            return -1;
        }

        string strPlaintText = "";
        nRc = LS_Decrypt_Watermark(
                    decryptData->digitalWatermarkingEncryptsData.toStdString(),
                    strSM4Key,
                    decryptData->privateKeyEncryptsData.toStdString(),
                    strPlaintText);
        if(nRc != 0)
        {
            qDebug()<<"strPlaintText error";
            return -2;
        }
        else if(strPlaintText.find("data:image") != 0)
        {
            qDebug()<<"strPlaintText not image";
            return -3;
        }

        // 去除 Base64 数据的前缀
            std::string base64Data = strPlaintText.substr(strPlaintText.find(",") + 1);
            decryptData->imageBase64Data = base64Data;

            // 解码 Base64 数据
            QByteArray byteArray(base64Data.c_str(), base64Data.length());
            QByteArray decodedData = QByteArray::fromBase64(byteArray);

            // 使用 QImage 加载解码后的字节数据
            QImage image;
            if (!image.loadFromData(decodedData)) {
                qDebug() << "Failed to load image from data.";
                return -4;
            }

            // 将 QImage 转换为 QPixmap
            QPixmap p = QPixmap::fromImage(image);

            // 使用 QPixmap::swap 进行复制
            out->swap(p);

        return 0;
    }

    int decryptQImage(DecryptData *decryptData,QImage *out){
        int nRc = 0;
        string strSM4Key = "";
        nRc = LS_Get_SM4Key(
                    decryptData->identification.toStdString(),
                    "listenVision",
                    strSM4Key);
        if(nRc != 0)
        {
            qDebug()<<"sm4Key error";
            return -1;
        }

        string strPlaintText = "";
        nRc = LS_Decrypt_Watermark(
                    decryptData->digitalWatermarkingEncryptsData.toStdString(),
                    strSM4Key,
                    decryptData->privateKeyEncryptsData.toStdString(),
                    strPlaintText);
        if(nRc != 0)
        {
            qDebug()<<"strPlaintText error";
            return -2;
        }
        else if(strPlaintText.find("data:image") != 0)
        {
            qDebug()<<"strPlaintText not image";
            return -3;
        }

        // 去除 Base64 数据的前缀
            std::string base64Data = strPlaintText.substr(strPlaintText.find(",") + 1);
            decryptData->imageBase64Data = base64Data;

            // 解码 Base64 数据
            QByteArray byteArray(base64Data.c_str(), base64Data.length());
            QByteArray decodedData = QByteArray::fromBase64(byteArray);

            // 使用 QImage 加载解码后的字节数据
            if (!out->loadFromData(decodedData)) {
                qDebug() << "Failed to load image from data.";
                return -4;
            }

        return 0;
    }

};

#endif // DECRYPTUTIL_H
