//资源模块头文件
#ifndef RESOURCE_H
#define RESOURCE_H

//判断文件是否存在， 即搜索资源
int searchResource(const char* path);
//判断请求文件类型， 为后续构建http响应头提供信息
//利用拓展名找mime.h中对应的类型
int identifyType(const char* path, char* type);
#endif // RESOURCE_H