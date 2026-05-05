//标准库相关头文件
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//系统调用相关的
#include <unistd.h>
#include <sys/stat.h> //mkfifo,文件元数据
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd = -1; //侦听套接字

int initSocket(){
    //创建套接字
    printf("pid: %d, tid: %p > Initializing socket...\n", getpid(), pthread_self());
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }
    //设置套接字
    //服务器重启时，可能会因为上次运行的套接字还未来得及释放，导致此次bind失败
    //所以需要设置套接字，保证重启正常, 用setsockopt函数设置套接字选项，允许地址重用
    printf("pid: %d, tid: %p > Setting up socket...\n");
    int on = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&on, sizeof(on)) == -1){
        perror("setsockopt");
        return -1;
    }
    //组织服务器地址结构
    ptintf("pid: %d, tid: %p > Setting up server address...\n", getpid(), pthread_self());
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    //一个服务器可以有多个网卡，对应着多个ip地址，INADDR_ANY表示任意ip地址
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //绑定
    printf("pid: %d, tid: %p > Binding socket...\n", getpid(), pthread_self());
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        return -1;
    }

    //侦听
    printf("pid: %d, tid: %p > Listening socket...\n", getpid(), pthread_self());
    if(listen(sockfd, 128) == -1){
        perror("listen");
        return -1;
    }
    return 0;
}

//接收客户端的连接请求,这是线程处理的事情吗？
int acceptClient(void){
    printf("pid: %d, tid: %p > Accepting client...\n", getpid(), pthread_self());
    struct sockaddr_in client_addr;
    //为什么一定要初始化这个变量？因为accept函数会修改这个变量，告诉我们客户端地址结构的实际长度，如果不初始化，可能会导致accept函数失败
    socklen_t client_addr_len = sizeof(client_addr); 
    int connect_fd = accept(sockfd, &client_addr, NULL);
    if(connect_fd == -1){
        perror("accept");
        return -1;
    }
    printf("pid: %d, tid: %p > Accepted connection from %s.%hu\n", getpid(), pthread_self(), 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    return connect_fd;
}

//接收http请求，多线程, buffer为接收到的数据
char* recvRequest(int connect_fd){
    //关键是设置存储区的大小， 理想的是请求内容有多少，存储区就开多少
    //方法是边接边存
    char* res; //请求内容存储区地址
    int len = 0; //存储区大小
    while(1){
        char buffer[1024] ={};
        ssize_t size = recv(connect_fd, buffer, sizeof(buffer) - 1, 0); //习惯留一字节给‘\0’
        if(size == -1){
            perror("recv");
            return NULL;
        }
        
        //扩大存储区，存储区的大小是当前已经存储的内容大小加上新接收的内容大小再加上1字节的‘\0’
        res = realloc(res, len + size + 1);
        if(res == NULL){
            perror("realloc");
            return NULL;
        }
        //拷贝新的内容， 指定拷贝字节数
        memcpy(res + len, buffer, sizeof(buffer));
        len += size;
        res[len] = '\0'; //添加字符串结束标志
    }

    return res;
}

//发送http响应头
int sendHead(int connect_fd, const char* response){
    if(send(connect_fd, response, strlen(response), 0) == -1){
        perror("send");
        return -1;
    }
    return 0;
}
//发送http响应体， path为要发送的文件路径
int sendBody(int connect_fd, const char* path){

}

//关闭套接字
int deinitSocket(void){
    close(sockfd);
    return 0;
}