//服务器模块头文件
#ifndef SERVER_H
#define SERVER_H

//初始化服务器, 创建套接字， 绑定端口，监听， 屏蔽信号
int initServer(short port);
//运行服务器接受客户端请求, 开线程，需要资源路径
int runServer(const char* path);
//关闭服务器
int closeServer();
#endif 