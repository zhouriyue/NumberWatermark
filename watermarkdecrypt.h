#ifndef WATERMARKDECRYPT_H
#define WATERMARKDECRYPT_H

#include <string>

#define LS_ERROR_LENGTH         0x1001  //长度错误
#define LS_ERROR_KEY_SM4        0x1002  //SM4密钥错误
#define LS_ERROR_KEY_PRIVATE    0x1003  //私钥错误
#define LS_ERROR_DECRYPT_SM2    0x1004  //SM2解密失败

extern "C"
{
///
/// \brief 计算SM4解密密钥
/// \param strMAC MAC地址
/// \param strSaltValue 盐值
/// \param strSM4Key 传出SM4解密密钥
/// \return 成功返回0；失败返回错误码
///
int LS_Get_SM4Key(
        std::string strMAC,
        std::string strSaltValue,
        std::string &strSM4Key);

///
/// \brief 解密水印数据
/// \param strEncryptData 传入水印数据密文BASE64码
/// \param strSM4Key 传入SM4解密密钥
/// \param strPrivateKey 传入私钥密文BASE64码
/// \param strPlaintText 传出水印数据明文
/// \return 成功返回0；失败返回错误码
///
int LS_Decrypt_Watermark(
        std::string strEncryptData,
        std::string strSM4Key,
        std::string strPrivateKey,
        std::string & strPlaintText);
}

#endif // WATERMARKDECRYPT_H
