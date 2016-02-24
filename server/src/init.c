#include "init.h"

extern int server_status;

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

void log_time(){
    time_t rtime;
    struct tm * ptm;
    time(&rtime);
    ptm = gmtime(&rtime);
    printf("%d年%d月%d日 %02d:%02d:%02d  ", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour + 8, ptm->tm_min, ptm->tm_sec);
}

void signal_handle(int signum){
    server_status = 0;
}
