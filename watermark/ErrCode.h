#pragma once

namespace nwe {

    //初始化
    enum Init {
        INIT_NULL_WATERMARK_CONTENT = 1001,  //水印内容为空
        INIT_WIDTH_ERROR = 1002,             //水印图片宽度错误
        INIT_HEIGHT_ERROR = 1003,           //水印图片高度错误
        INIT_TRANS_GRAY_ERROR = 1004,          //转换为灰度失败
        INIT_SUCCESS = 0       //初始化成功
    };

    //嵌入方法
    enum INSERT_WATERMARK {
        INSERT_NULL_SOURCE_CONTENT = 2001,       //嵌入图片为空
        INSERT_WIDTH_ERROR = 2002,          //嵌入图片宽度错误
        INSERT_HEIGHT_ERROR = 2003,      //嵌入水印图片高度错误
        INSERT_SOURCE_TRANS_MAT_ERROR = 2004,      //源图片转换成Mat错误
        INSERT_BLOCK_SIZE = 2005,           //分块大小错误
        INSERT_BLOCK_TRANS_MAT_ERROR = 2006, //分块转换成Mat错误
        INSERT_RESULT_TRANS_MAT_ERROR = 2007, //结果转换成Mat错误
        INSERT_SUCCESS = 0 //添加成功
    };

    //获取水印
    enum GET_WATERMARK {
        GET_NULL_SOURCE_CONTENT = 3001,   //被嵌入图片内容为空
        GET_WIDTH_ERROR = 3002,       //被嵌入图片宽度错误
        GET_HEIGHT_ERROR = 3003,      //被嵌入图片高度错误
        GET_INSERT_SOURCE_TRANS_ERROR = 3004, //被嵌入图片转换失败
        GET_BLOCK_SIZE_ERROR = 3005,//分快大小错误
        GET_BLOCK_TRANS_MAT_ERROR = 3006, //分块转换成Mat错误
        GET_SUCCESS = 0, //提取成功
    };

    enum MSE {
        MES_ERROR_IMG1_WIDTH_ERROR = 4001,  //图片1宽度错误
        MES_ERROR_IMG1_HEIGHT_ERROR = 4002,  //图片1高度错误
        MES_ERROR_IMG2_WIDTH_ERROR = 4003,  //图片2宽度错误
        MES_ERROR_IMG2_HEIGHT_ERROR = 4004,  //图片2高度错误
        MES_ERROR_SIZE_IMG1_NOT_EQUAL_IMG2 = 4005, //图片1大小不等于图片2
        MES_ERROR_TYPE_IMG1_NOT_EQUAL_IMG2 = 4006, //图片1类型不等于图片2
        MES_SUCCESS = 0,    //比较成功
    };
};
