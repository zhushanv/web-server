//信号模块头文件
#ifndef SIGNAL_H
#define SIGNAL_H

//初始化信号, 屏蔽掉大部分信号， 但是留下2号信号SIGINT和15号信号SIGTERM， 以便在需要时能够正常关闭服务器
int init_signal(void);
#endif //SIGNAL_H