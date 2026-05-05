//http模块实现
#include "http.h"
#include <sys/syscall.h>
#include <unistd.h> //NULL
#define __USE_GNU //为了使用strcasestr函数
#include <string.h> 
#include <time.h> //strftime
#include <stdio.h> //printf, sprintf
#include <pthread.h> //pthread_self

//解析http请求
//sscanf从字符串中读取数据，格式化输入
int parseRequest(const char* request, HTTP_REQUEST* hreq){
    //解析请求行
    sscanf(request, "%s %s %s", hreq->method, hreq->path, hreq->version);

    char* connection = strcasestr(request, "Connection: ");
    if(connection != NULL){
        sscanf(connection, "Connection: %s", hreq->connection);
    }

   //进程id， 线程id
   printf("pid: %d, tid: %p > [%s][%s][%s][%s]\n", getpid(), pthread_self(), hreq->method, hreq->path, hreq->version, hreq->connection);
   
   //判断请求方法
   if(strcasecmp(hreq->method, "get")){
        printf("pid: %d, tid: %p > Unsupported HTTP method: %s\n", getpid(), pthread_self(), hreq->method);
        return -1;
   }
   //判断协议版本
   if(strcasecmp(hreq->version, "HTTP/1.1") && strcasecmp(hreq->version, "HTTP/1.0")){
        printf("pid: %d, tid: %p > Unsupported HTTP version: %s\n", getpid(), pthread_self(), hreq->version);
        return -1;
   }
   return 0;
}

//构建http响应
int constructHead(const HTTP_RESPONSE* hres, char* response){
    char timebuf[128];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(timebuf, sizeof(timebuf), "%a %d %b %Y %T", tm_info);

    sprintf(response, "%s %d %s\r\n"
                      "Server: ShanziServer 1.0\r\n"
                      "Date: %s\r\n"
                      "Content-Type: %s\r\n"
                      "Content-Length: %lld\r\n"
                      "Connection: %s\r\n\r\n",
                      hres->version, hres->status, hres->desc, timebuf
                      , hres->type, hres->length, hres->connection);
    return 0;
}