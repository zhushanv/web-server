//服务器模块实现
#include "server.h"
#include "client.h"
#include "socket.h"
#include "signals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int initServer(short port){
    //初始化信号
    if(init_signal() == -1){
        return -1;
    }

    //初始化socket
    if(initSocket(port) == -1){
        return -1;
    }

    return 0;
}

int runServer(const char* path){
    //循环接受客户端的连接
    while(1){
        int conn = acceptClient();
        if(conn == -1){
            return -1;
        }

        //创建线程处理客户端请求
        CA *ca = (CA*)malloc(sizeof(CA)); //动态分配维持生命周期
        ca->conn = conn;
        ca->home = path;
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr); //一定要初始化属性，不让会有一些奇怪的问题
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //设置线程为分离状态
        //考虑线程资源的回收，用pthread_detach或者资源属性一步到位，可以实现非阻塞的系统自动回收
        int error = pthread_create(&tid, &attr, client_thread, (void*)ca);
        if(error){
            fprintf(stderr, "create thread error: %s\n", strerror(error));
            return -1;
        }
    }
    return 0;
}

//关闭服务器
int closeServer(){
    return deinitSocket();
}