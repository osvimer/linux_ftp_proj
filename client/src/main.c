#include "trans_file.h"
#include "identify.h"
#include "init.h"
#include "cmd.h"

#define LOG_FILE "/tmp/ftp_client.log"
int sfd;
int connect_status = 1;

int main (int argc, char const * argv[]){
    if(argc != 3){
        printf("args error!\n");
        return -1;
    }
    int log_fd = open(LOG_FILE, O_WRONLY|O_CREAT, 0666);
    lseek(log_fd, 0, SEEK_END);
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
    char logtime[24] = {0};
    int status, trans_fd;
    long int file_len, offset;
    struct stat file_stat;
    //身份认证
    get_identify(sfd, username);
    //记录登录信息到日志
    log_time(logtime);
    bzero(&buf, sizeof(buf));
    sprintf(buf.data, "%s  %s connect to %s succeess\n", logtime, username, argv[1]);
    write(log_fd, buf.data, strlen(buf.data));
    //接收服务端当前目录
    recv_complete(sfd, &buf);;
    strcpy(pwd, buf.data);
    char args[3][20] = {0};
    connect_status = 1;
    system("clear");
    while(connect_status){
        //打印命令提示符
        printf("%s @ %s in %s $ ", username, argv[1], pwd);
        fflush(stdout);
        //读取用户输入，分解命令及参数
        bzero(&buf, sizeof(buf));
        bzero(args, sizeof(args));
        if((ret = read(STDIN_FILENO, buf.data, sizeof(buf.data))) > 0){
            sscanf(buf.data, "%s%s%s", args[0], args[1], args[2]);
            //记录用户操作到日志
            log_time(logtime);
            bzero(&buf, sizeof(buf));
            sprintf(buf.data, "%s  %s %s %s\n", logtime, args[0], args[1], args[2]);
            write(log_fd, buf.data, strlen(buf.data));
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
                //发送文件偏移大小
                status = stat(args[1], &file_stat);
                if(-1 == status){
                    offset = 0;
                }else{
                    offset = file_stat.st_size;
                }
                send(sfd, &offset, sizeof(long), 0);
                //接收服务端响应状态
                recv_status(sfd, &status);
                if(-1 == status){
                    printf("gets failed!\n");
                    continue;
                }
                //获取文件大小
                recv(sfd, &file_len, sizeof(int), 0);
                trans_fd = open(args[1], O_WRONLY|O_CREAT, 0644);
                //重定位到文件末尾
                lseek(trans_fd, 0, SEEK_END);
                //获取文件内容
                recv_file_by_fd(sfd, trans_fd, file_len - offset);
                close(trans_fd);
                printf("download success!\n");
            }else if(0 == strcmp(args[0], "puts")){
                //判断文件状态
                status = stat(args[1], &file_stat);
                if(-1 == status){
                    printf("file not exist!\n");
                    continue;
                }
                strcpy(buf.data, "STOR ");
                strcat(buf.data, args[1]);
                buf.len = strlen(buf.data);
                send_complete(sfd, &buf);
                //接受文件偏移大小
                recv(sfd, &offset, sizeof(long), 0);
                file_len = file_stat.st_size;
                if(offset >= file_len){
                    send_status(sfd, -1);
                    printf("puts failed!\n");
                    continue;
                }
                send_status(sfd, 0);
                //发送文件大小
                send(sfd, &file_len, sizeof(long), 0);
                trans_fd = open(args[1], O_RDONLY);
                //重定位到续传位置
                lseek(trans_fd, offset, SEEK_SET);
                //发送文件内容
                if(file_len > FILE_MMAP_SIZE){//大文件采取内存映射
                    int i;
                    long j = 0;
                    long send_len = file_len - offset;
                    //char * mpt = (char *)mmap(0, send_len, PROT_READ, MAP_SHARED, trans_fd, offset);
                    char * mpt = (char *)mmap(0, send_len, PROT_READ, MAP_SHARED, trans_fd, 0);
                    while(j < send_len){
                        bzero(&buf, sizeof(buf));
                        for(i = 0; i < sizeof(buf.data) && j < send_len; i++, j++){
                            buf.data[i] = mpt[j];
                        }
                        buf.len = i;
                        send_complete(sfd, &buf);
                    }
                    munmap(mpt, send_len);
                }else{//小文件直接发送
                    send_file_by_fd(sfd, trans_fd);
                }
                close(trans_fd);
                printf("upload success!\n");
            }else if(0 == strcmp(args[0], "exit")){
                strcpy(buf.data, "ABORT");
                buf.len = strlen(buf.data);
                send_complete(sfd, &buf);
                connect_status = 0;
            }else{
                printf("grammar error!\n");
                continue;
            }
        }else{
            break;
        }
    }
    log_time(logtime);
    bzero(&buf, sizeof(buf));
    sprintf(buf.data, "%s  close connect\n", logtime);
    write(log_fd, buf.data, strlen(buf.data));
    close(sfd);
    close(log_fd);
    return 0;
}
