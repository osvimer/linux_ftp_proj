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
            //命令处理：按命令是否涉及文件及目录操作，分为界面交互命令（clear, exit）和文件操作命令; 文件操作命令按是否以"!"开头区分为本地命令和远程命令
            //本地命令：为满足本地文件及目录操作需要，同时避免复杂化，限制用户使用以下本地命令: ls/cd/pwd/rm/mkdir/mv/du。除"cd"调用chdir外，其他直接调用同名系统命令。
            if('!' == args[0][0]){
                if(0 == strcmp(args[0], "!cd")){
                    chdir(args[1]);
                }else if((0 == strcmp(args[0], "!ls")) || (0 == strcmp(args[0], "!pwd")) || (0 == strcmp(args[0], "!rm")) || (0 == strcmp(args[0], "!mkdir")) || (0 == strcmp(args[0], "!mv")) || (0 == strcmp(args[0], "!du"))){
                    char * ptr = args[0];
                    sprintf(buf.data, "%s %s %s", ptr + 1, args[1], args[2]);
                    system(buf.data);
                }else{
                    printf("grammar error!\n");
                }
                continue;
            }
            //远程命令: 1.发送命令及参数 2.接收服务端响应码 3.接受/发送数据(可选)
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
            }
            //界面交互命令
            else if(0 == strcmp(args[0], "help")){
                printf("help\t\t\t--获取帮助\nclear\t\t\t--清理终端\nexit\t\t\t--退出客户端\n\n!ls LOCAL_DIR/FILE\t--列出本地目录/文件\n!cd LOCAL_DIR\t\t--切换本地目录\n!pwd\t\t\t--打印本地当前路径\n!rm LOCAL_FILE\t\t--删除本地文件\n!mkdir LOCAL_DIR\t--创建本地目录\n!mv SRC_PATH DST_PATH\t--移动本地目录/文件\n!du LOCAL_FILE\t\t--查看本地目录/文件大小\n\nls REMOTE_DIR/FILE\t--列出远程目录/文件\npwd\t\t\t--打印服务端当前路径\ncd REMOTE_DIR\t\t--切换服务端目录\nrm REMOTE_FILE\t\t--删除服务端文件\ngets REMOTE_FILE\t--下载服务端文件到本地\nputs LOCAL_FILE\t\t--上传本地文件到服务端\n");
                continue;
            }else if(0 == strcmp(args[0], "clear")){
                system(args[0]);
                continue;
            }else if(0 == strcmp(args[0], "exit")){
                strcpy(buf.data, "ABORT");
                buf.len = strlen(buf.data);
                send_complete(sfd, &buf);
                connect_status = 0;
            }
            //无效命令
            else{
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
