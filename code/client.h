#ifndef CLIENT_H
#define CLIENT_H

//线程参数类型
typedef struct client_arg{
    int conn;  //客户端套接字
    char* home; //资源路径
}CA;
//线程过程函数，负责和客户端通信
//不断响应客户端的请求，直到客户端断开连接
void* client_thread(void *arg);

#endif // CLIENT_H