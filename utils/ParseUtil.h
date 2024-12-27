#ifndef PARSEUTIL_H
#define PARSEUTIL_H
#include <QString>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "entity/DecryptData.h"

class ParseUtil{

public:
    int parseDecryptData(QString filePath,DecryptData *decryptData){
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly)){
            qDebug()<<"无法打开文件 "<<filePath;
            return -1;
        }
        QByteArray jsonData = file.readAll();
        file.close();

        //解析出Json数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if(jsonDoc.isNull()){
            qDebug()<<"解析json失败";
            return -1;
        }

        QJsonObject jsonObj = jsonDoc.object();
        decryptData->identification = jsonObj["identification"].toString();
        decryptData->asymmetricEncryptionType = jsonObj["asymmetricEncryptionType"].toInt();
        decryptData->privateKeyEncryptsData = jsonObj["privateKeyEncryptsData"].toString();
        decryptData->digitalWatermarkingEncryptsData = jsonObj["digitalWatermarkingEncryptsData"].toString();
        decryptData->expirationTime = jsonObj["expirationTime"].toInt();
        return 0;
    }

};

#endif // PARSEUTIL_H
