#ifndef DECRYPTDATA_H
#define DECRYPTDATA_H

#include <QString>
#include <string>

using namespace std;

class DecryptData{

public:
    QString identification;//MAC地址
    QString expirationTime;//盐值
    int asymmetricEncryptionType = 0;//加密类型
    QString privateKeyEncryptsData;//私钥
    QString digitalWatermarkingEncryptsData;//加密水印数据
    string imageBase64Data;//图片base64数据
};

#endif // DECRYPTDATA_H
