//http头文件
#ifndef HTTP_H
#define HTTP_H

#include <limits.h>
#include <sys/types.h> //定义了PATH_MAX
//解析得到的关键数据
typedef struct httpRequest{
    char method[16]; //请求方法
    char path[PATH_MAX];  //请求路径， 不清楚请求路径的具体长度，就用系统定义的最大路径长度
    char version[32]; //HTTP版本号
    char connection[32]; //要么是"keep-alive"，要么是"close"
}HTTP_REQUEST;

typedef struct httpResponse{
    char version[32]; //
    int status;
    char desc[64];//状态描述符
    char type[64]; //返回的内容类型
    int length; //返回的内容长度
    char connection[32]; //连接状态
}HTTP_RESPONSE;

//函数一， 解析http请求，返回请求的资源路径(用输出型参数表示)
//成功返回0，失败返回-1
int parseRequest(const char* request, HTTP_REQUEST* hreq);
//函数二， 构造http响应，返回响应字符串
//一般返回值用来表示是否成功，成功返回0，失败返回-1，要存储的内容用输出型参数表示
int constructHead(const HTTP_RESPONSE* hres, char* response);
#endif // HTTP_H