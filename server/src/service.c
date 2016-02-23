#include "service.h"
#include "list.h"
#include "cmd.h"
#include "identify.h"
#include "trans_fd.h"
#include "trans_file.h"

extern int server_status;

void service_handle(int child_fd){
    int task_fd;
    int status = 0;
    char args[3][20] = {0};
    data_t buf;
    int connect_status = 0;
    //开始服务
    while(server_status){
        recv_fd(child_fd, &task_fd);//如果任务没到，会阻塞
        printf("start identify.\n");
        client_identify(task_fd);
        printf("start accept command!\n");
        //发送当前工作目录
        send_pwd(task_fd);
        //开始接受并处理服务端命令
        connect_status = 1;
        while(connect_status){
            //接收服务端命令
            bzero(&buf, sizeof(buf));
            status = recv(task_fd, &buf, sizeof(buf), 0);
            if(0 == status){
                connect_status = 0;
            }
            //分解命令及参数
            sscanf(buf.data, "%s%s%s", args[0], args[1], args[2]);
            //处理命令
            if(strcmp(args[0], "LIST") == 0){
                send_list(task_fd, args[1]);
            }else if(strcmp(args[0], "CWD") == 0){
                status = chdir(args[1]);
                send_status(task_fd, status);
                send_pwd(task_fd);
            }else if(strcmp(args[0], "PWD") == 0){
                send_pwd(task_fd);
            }else if(strcmp(args[0], "RMD") == 0){
                status = remove(args[1]);
                send_status(task_fd, status);
            }else if(strcmp(args[0], "RETR") == 0){
                send_file_by_name(task_fd, args[1]);
            }else if(strcmp(args[0], "STOR") == 0){
                recv_file_by_name(task_fd, args[1]);
            }else if(strcmp(args[0], "ABORT") == 0){
                connect_status = 0;
            }
        }
        //任务完成, 关闭task_fd，通知主进程(让其标识为空闲)
        close(task_fd);
        send_status(child_fd, 0);
    }
}
