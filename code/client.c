//客户机线程
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> //free
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

#include "client.h"
#include "http.h"
#include "resource.h"
#include "socket.h"

//主程度负责连接和参数的构建
//参数用malloc分配地址，保证有足够的生命周期，只不过要线程来释放内存，🙅忘记
void *client_thread(void *arg){
    CA* ca = (CA*)arg;
    int conn = ca->conn;
    char* home = ca->home;
    printf("pid: %d, tid: %p > Client thread begin....\n", getpid(), pthread_self());
    //接收请求信息
    while(1){
        printf("pid: %d, tid: %p > Client thread loop....\n", getpid(), pthread_self());
        //拿到客户端发来的所有请求内容
        //reques所指向的存储区是malloc分配的，需要手动释放内存
        char* request = recvRequest(conn);
        if(request == NULL){
            break;
        }
        //显示请求内容
        printf("pid: %d, tid: %p > Client thread request: \n%s\n", getpid(), pthread_self(), request);
        
        //处理请求
        printf("pid: %d, tid: %p > Client thread parse request....\n", getpid(), pthread_self());
        //用来存储解析得到的数据
        HTTP_REQUEST hreq = {};
        if(parseRequest(request, &hreq) == -1){
            free(request);
            break;
        }
        //request内容已经被解析了，它已经没有利用价值了，释放内存
        free(request);
        //寻找资源
        //先构造正确的访问路径， 去掉资源路径后可能存在的/
        if(ca->home[strlen(ca->home)-1] == '/'){
            ca->home[strlen(ca->home)-1] = '\0';
        }

        //特殊情况--根目录/ , 这是客户端为指定特定文件， 我们默认为首页文件index.html
        if(strcmp(hreq.path, "/") == 0){
            strcpy(hreq.path, "/index.html");
        }

        //路径拼接
        char path[PATH_MAX] = {};
        snprintf(path, PATH_MAX, "%s%s", ca->home, hreq.path);
        printf("pid: %d, tid: %p > Client thread search resource path: %s\n", getpid(), pthread_self(), path);
        
        //搜索资源,构造响应头,初始化为常用的值
        HTTP_RESPONSE hres = {"HTTP/1.1", 200, "OK", "text/html"};

        if(searchResource(path) == -1 || identifyType(path, hres.type) == -1){
            hres.status = 404;
            strcpy(hres.desc, "Not Found");
            //更新path到404.html
            strcpy(path, home);
            strcat(path, "/404.html");
        }

        //响应文件的大小， 用文件元数据
        struct stat st;
        if(stat(path, &st) == -1){
            perror("stat");
            break;
        }
        hres.length = st.st_size;

        //连接状态分客户端是否有连接状态这两种情况
        //因为 connection 是一个字符数组，不是指针。数组名在表达式里会退化成首元素地址，这个
        //地址不可能是 NULL。hreq.connection == NULL这个判断条件是错误的
        if(hreq.connection[0] == '\0'){
            //根据请求的协议版本来定
            if(strcmp(hreq.version, "HTTP/1.1") == 0){
                strcpy(hres.connection, "keep-alive");
            }else{
                strcpy(hres.connection, "close");
            }
        }else{
            //同客户端保持一次
            strcpy(hres.connection, hreq.connection);
        }
        //构造响应头
        printf("pid: %d, tid: %p > Client thread construct response header....\n", getpid(), pthread_self());
        char response[1024] = {};
        if(constructHead(&hres, response) == -1){
            break;
        }
        printf("pid: %d, tid: %p > Client thread response header: \n%s\n", getpid(), pthread_self(), response);
        //发送响应头
        printf("pid: %d, tid: %p > Client thread send response header....\n", getpid(), pthread_self());
        if(sendHead(conn, response) == -1){
            break;
        }
        //发送响应体
        printf("pid: %d, tid: %p > Client thread send response body....\n", getpid(), pthread_self());
        if(sendBody(conn, path) == -1){
            break;
        }
        //根据连接状态判断需不需要进入下一次循环
        if(strcasecmp(hres.connection, "close") == 0){
            break;
        }
    }
    //关闭文件描述符， 释放动态分配内存
    close(conn);
    free(ca);
    printf("pid: %d, tid: %p > Client thread end....\n", getpid(), pthread_self());
    return NULL;
}