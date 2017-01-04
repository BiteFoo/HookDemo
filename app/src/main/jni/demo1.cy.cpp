//
// Created by Administrator on 2016/12/29.
//
//substrate 需要的头文件
#include "substrate.h"
//Android ndk 开发的头文件
#include "jni.h"
//Android 日志打印头文件
#include "android/log.h"
//C 语言标准库
#include "stdio.h"
//C语言文件操作库
#include "fcntl.h"

#include "sys/types.h"
#include "string.h"
#include "sys/stat.h"
#include "unistd.h"

//使用substrate hook C代码，首先是要使用MSConfig(__substrate__,char *name)函数
//MSConfig(name,value)，函数定义的形式，name:要过滤的条件，lib ,executable ,value：名称，例如
//某一个文件的名称等 libxxx.so 或者是 xxx进程，通常选择的是 /system/bin/app_process(绝对路径下的)
//或者是想要注入的进程，一般根据报名来注入， /data/data/com.xxx.xxx等的形式

/**
总结方式：
使用MSFilterLibrary,libname
使用MSFilterExecutable,processName


*/
/**
这里通常配置第一个参数：
MSFilterLibrary : "FilterLibrary" 过滤出库文件
MSFilterExecutable: "FilterExecutable" 过滤process
第二个参数，通常是要hook住的libxxx.so文件名称 或者是processName,根据/data/data/com.xx.xxx的形式存在

*/
//定义打印日志接口
#define TAG "demo1"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
//测试1，选择日志文件查看代码
MSConfig(MSFilterLibrary,"libdvm.so")//hook libdvm的日志

bool (*_dvmLoadNativeCode)(char* pathName,void* classLoader,char** detail);//要hook的函数,声明出来

bool fake_dvmLoadNativeCode(char* soPath,void* classLoader,char** detail) //要实现的函数，就是自己的函数
{
//自己定义的函数，通过hook住制定函数，然函数执行我们自己的流程，返回函数需要的值就可以，
//中间可以包括记录下执行函数参数的和执行得到的结果等行为，同时也可以写文件等等
//也可以通过了解制定函数的执行过程，在指定位置截取出需要的信息
LOGD("in my function ,do my logic ,hook func arg : soPathName=%s ",soPath);
return _dvmLoadNativeCode(soPath,classLoader,detail);//还是让参数知己做，把关键的参数留下，

}
#define hookSoName "/system/lib/libdvm.so"
//在配置完成，和写好要hook程序之后，调用入口函数
//入口函数
MSInitialize{
//提示信息
LOGD("substrate main entry point initialize");


//在进入到函数内部之后，第一步，找到函数的地址，或者是句柄，
/*
根据官网的提示，接收句柄操作的类
MSImageRef image
获取函数
MSGetImageByName( const char* name) ,return MSImageRef or NULL

void* MSFindSymbol(MSImageRef image,const char*funcName) 这里就是根据得到的so句柄，得到指定的函数句柄

void MSHookFunction(void* symbol,void*hook ,void** old)

symbol:就是根据指定的so得到制定函数的句柄
hook:就是自己的函数，可代替或者不代替old函数功能，让old执行自己的逻辑，
old:指定函数(自己想要操作的函数)

*/
MSImageRef image;
image=MSGetImageByName(hookSoName);
if(image == NULL)
{
LOGD("can't hook the so of  %s",hookSoName);
return ;
}
LOGD("got it dvm image: 0x%08X",(void*)image);

//得到指定函数，通过FindSymbol函数
void* oldsymbol=MSFindSymbol(image,"_Z17dvmLoadNativeCodePKcP6ObjectPPc");
if(oldsymbol == NULL)
{
LOGD(" Can't get the function of we need ,oldsymbol,and return now @@！！");
return;
}
//接着，操作这个句柄，通过调用MSHookFunction(void* image,void *hook,void* old),实现指定函数功能
MSHookFunction(oldsymbol,(void*)&fake_dvmLoadNativeCode,(void**)&_dvmLoadNativeCode);
//不理解位置，为什么加了void* 之后，还要加上&,有何作用？？
//解决： 这里需要把函数的地址传递进去,需要注意，每个函数参数类型
LOGD("HOO HOO hook finished !!");

//完成操作

}