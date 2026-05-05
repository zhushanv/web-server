#include "server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    //命令行./a.out 8080 /home/xxx/
    //命令行指定端口号和根目录，未指定则使用默认值
    int port = 80; //默认端口号
    if(argc >= 2){
        port = atoi(argv[1]);
    }
    //初始化服务器, 需要命令行的端口号
    if(initServer(port) == -1){
        printf("初始化服务器失败\n");
        return -1;
    }
    //启动服务器
    char* root = "../home";
    if(argc >= 3){
        root = argv[2];
    }
    if(runServer(root) == -1){
        printf("启动服务器失败\n");
        return -1;
    }

    if(closeServer() == -1){
        printf("关闭服务器失败\n");
        return -1;
    }
    return 0;
}