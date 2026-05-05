//通信模块头文件
#ifndef SOCKET_H
#define SOCKET_H

//初始化套接字
int initSocket(short port);

//接受客户端的连接请求
int acceptClient(void);

//接收http请求，多线程, buffer为接收到的数据
char* recvRequest(int connect_fd);

//发送http响应头
int sendHead(int connect_fd, const char* response);
//发送http响应体， path为要发送的文件路径
int sendBody(int connect_fd, const char* path);

//关闭套接字
int deinitSocket(void);
#endif //SOCKET_H