#include "FFmpegInclude.h"

///
/// \brief 非阻塞延时
/// \param msec 延时毫秒
///
void ffsleepMsec(int msec)
{
    if(msec <= 0) return;
    QEventLoop loop;		//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();			//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
