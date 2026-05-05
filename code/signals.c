#include "signals.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h> 
#include <pthread.h> //pthread_self函数

//目前的信号编号的最大值
int init_signal(void){
    printf("pid: %d, tid: %p > Initializing signals\n", getpid(), pthread_self());
    for(int i = 1; i < NSIG; i++){
        if(i != SIGINT && i != SIGTERM){
            signal(i, SIG_IGN); //有的信号不能忽略，如果错误处理的话，会报错， 这里忽略掉
        }
    }
    return 0; 
}
