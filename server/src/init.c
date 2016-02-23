#include "init.h"

void set_daemon(){
    const int MAXFD = 64;
    if(fork() != 0){//父进程退出
        exit(0);
    }
    setsid();//成为新进程组组长和新会话领导，脱离控制终端
    //chdir("/");//设置工作目录为根目录
    umask(0);//重设文件访问权限掩码
    int i;
    for(i = 0; i < MAXFD; i++){//尽可能关闭所有从父进程继承来的文件
        close(i);
    }
}

void signal_handle(int signum){
    exit(0);
}
