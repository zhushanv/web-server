#include "resource.h"
#include "mime.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h> //access函数
#include <pthread.h> //pthread_self函数

int searchResource(const char* path){
    //acceses函数同时判断文件是否存在以及是否有可读权限
    return access(path, R_OK);
}

int identifyType(const char* path, char* type){
    //提取拓展名，大串当中找char， 最后一个‘.’
    char* dot = strrchr(path, '.');
    if(dot == NULL){
        return -1;
    }
    
    for(int i = 0; i < sizeof(s_mime)/sizeof(s_mime[0]); i++){
        if(strcasecmp(s_mime[i].suffix, dot) == 0){
            strcpy(type, s_mime[i].type);
            return 0;
        }
    }

    printf("pid: %d, tid: %p >  Can't find the file type\n", getpid(), pthread_self());
    return -1;
}