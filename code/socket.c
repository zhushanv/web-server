//标准库相关头文件
#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//系统调用相关的
#include <unistd.h>
#include <fcntl.h> //文件操作
#include <sys/stat.h> //mkfifo,文件元数据
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd = -1; //侦听套接字

int initSocket(short port){
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
    printf("pid: %d, tid: %p > Setting up socket...\n", getpid(), pthread_self());
    int on = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&on, sizeof(on)) == -1){
        perror("setsockopt");
        return -1;
    }
    //组织服务器地址结构
    printf("pid: %d, tid: %p > Setting up server address...\n", getpid(), pthread_self());
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
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
    int connect_fd = accept(sockfd, (struct sockaddr*)&client_addr, NULL);
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
    //只不过这样子要时刻注意存储区的内存管理，防止内存泄漏

    char* res; //请求内容存储区地址
    int len = 0; //存储区大小

    //用size = 0 判断循环结束表示的是对方关闭通信套接字
    //这里需要根据对方请求信息的结尾来判断，对方请求信息以\r\n\r\n结尾
    while(1){
        char buffer[1024] ={};
        ssize_t size = recv(connect_fd, buffer, sizeof(buffer) - 1, 0); //习惯留一字节给‘\0’
        if(size == -1){
            free(res);
            perror("recv");
            return NULL;
        }
        if(size == 0){
            printf("pid: %d, tid: %p > Client closed connection.\n", getpid(), pthread_self());
            free(res);
            return NULL;
        }
        //扩大存储区，存储区的大小是当前已经存储的内容大小加上新接收的内容大小再加上1字节的‘\0’
        res = realloc(res, len + size + 1);
        if(res == NULL){
            free(res);
            perror("realloc");
            return NULL;
        }
        //拷贝新的内容， 指定拷贝字节数
        memcpy(res + len, buffer, size);
        len += size;
        res[len] = '\0'; //添加字符串结束标志
        
        //大串里面找小串
        if(strstr(res, "\r\n\r\n") != NULL){
            break;
        }
    }

    return res;
}

//发送http响应头, 响应头的内容由参数传入
int sendHead(int connect_fd, const char* response){
    //注意发送的是有效字符串，要用strlen计算有效字符串长度
    if(send(connect_fd, response, strlen(response), 0) == -1){
        perror("send");
        return -1;
    }
    return 0;
}
//发送http响应体， path为要发送的文件路径
//打开文件， 读取文件，发送文件内容， 关闭文件
//可以试图找到文件的大小，比如说用stat元数据， 或者lseek设置读写位置到文件尾
int sendBody(int connect_fd, const char* path){
    int fd = open(path, O_RDONLY);
    //存储区大小不确定就循环接收
    char buffer[1024];
    ssize_t len;
    while((len = read(fd, buffer, sizeof(buffer) - 1)) > 0){
        if(send(connect_fd, buffer, len, 0) == -1){
            perror("send");
            close(fd);
            return -1;
        }
    }
    close(fd);
    if(len == -1){
        perror("read");
        return -1;
    }

    return 0;
}

//关闭套接字
int deinitSocket(void){
    close(sockfd);
    return 0;
}