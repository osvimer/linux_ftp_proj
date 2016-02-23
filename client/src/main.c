#include "trans_file.h"
#include "identify.h"
#include "init.h"
#include "cmd.h"

int sfd;

int main (int argc, char const * argv[]){
    if(argc != 3){
        printf("args error!\n");
        return -1;
    }
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfd){
        perror("socket");
        return -1;
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    int ret = connect(sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if(-1 == ret){
        perror("connect");
        return -1;
    }
    signal(SIGINT, signal_handle);
    data_t buf;
    char username[20] = {0};
    char pwd[128] = {0};
    int status, trans_fd;
    long int file_len;
    //身份认证
    get_identify(sfd, username);
    //接收服务端当前目录
    recv_complete(sfd, &buf);;
    strcpy(pwd, buf.data);
    char args[3][20] = {0};
    int connect_status = 1;
    system("clear");
    while(connect_status){
        //打印命令提示符
        printf("%s @ %s in %s $ ", username, argv[1], pwd);
        fflush(stdout);
        //读取用户输入，分解命令及参数
        bzero(&buf, sizeof(buf));
        bzero(args, sizeof(args));
        read(STDIN_FILENO, buf.data, sizeof(buf.data));
        sscanf(buf.data, "%s%s%s", args[0], args[1], args[2]);
        bzero(&buf, sizeof(buf));
        //处理命令: 1.发送命令及参数 2.接收服务端响应码 3.接受/发送数据(可选)
        if(0 == strcmp(args[0], "ls")){
            strcpy(buf.data, "LIST ");
            if(0 == strlen(args[1])){
                args[1][0] = '.';
            }
            strcat(buf.data, args[1]);
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_status(sfd, &status);
            if(-1 == status){
                printf("list failed!\n");
                continue;
            }
            //获取目录文件信息
            recv_list(sfd);
        }else if(0 == strcmp(args[0], "cd")){
            strcpy(buf.data, "CWD ");
            if(0 == strlen(args[1])){
                args[1][0] = '.';
            }
            strcat(buf.data, args[1]);
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_status(sfd, &status);
            if(-1 == status){
                printf("cd failed!\n");
                continue;
            }
            bzero(&buf, sizeof(buf));
            recv_complete(sfd, &buf);
            strcpy(pwd, buf.data);
        }else if(0 == strcmp(args[0], "pwd")){
            strcpy(buf.data, "PWD");
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_complete(sfd, &buf);
            strcpy(pwd, buf.data);
            printf("%s\n", pwd);
        }else if(0 == strcmp(args[0], "rm")){
            strcpy(buf.data, "RMD ");
            strcat(buf.data, args[1]);
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_status(sfd, &status);
            if(-1 == status){
                printf("remove failed!\n");
                continue;
            }
            printf("remove success!\n");
        }else if(0 == strcmp(args[0], "gets")){
            strcpy(buf.data, "RETR ");
            strcat(buf.data, args[1]);
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_status(sfd, &status);
            if(-1 == status){
                printf("gets failed!\n");
                continue;
            }
            //获取文件大小
            recv_file_len(sfd, &file_len);
            //获取文件内容
            trans_fd = open(args[1], O_WRONLY|O_CREAT, 0644);
            recv_file_by_fd(sfd, trans_fd, file_len);
            close(trans_fd);
            printf("download success!\n");
        }else if(0 == strcmp(args[0], "puts")){
            //判断文件状态
            trans_fd = open(args[1], O_RDONLY);
            if(-1 == trans_fd){
                printf("file not exist!\n");
                continue;
            }
            strcpy(buf.data, "STOR ");
            strcat(buf.data, args[1]);
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            recv_status(sfd, &status);
            if(-1 == status){
                printf("puts failed!\n");
                continue;
            }
            close(trans_fd);
            //发送文件大小
            send_file_len(sfd, args[1]);
            //发送文件内容
            trans_fd = open(args[1], O_RDONLY);
            send_file_by_fd(sfd, trans_fd);
            close(trans_fd);
            printf("upload success!\n");
        }else if(0 == strcmp(args[0], "exit")){
            strcpy(buf.data, "ABORT");
            buf.len = strlen(buf.data);
            send_complete(sfd, &buf);
            exit(0);
        }else{
            printf("grammar error!\n");
            continue;
        }
    }
    close(sfd);
    return 0;
}
