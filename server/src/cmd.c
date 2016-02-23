#include "head.h"
#include "list.h"
#include "cmd.h"
#include "trans_file.h"

#define TMP_FILE "/tmp/_ftp_ls.tmp"

void send_pwd(int sfd){
    data_t buf;
    bzero(&buf, sizeof(buf));
    getcwd(buf.data, sizeof(buf.data));
    buf.len = strlen(buf.data);
    send_complete(sfd, &buf);
}

int send_list(int sfd, char * file_name){
    struct stat file_stat;
    int status = stat(file_name, &file_stat);
    if(-1 == status){
        send_status(sfd, -1);
        return -1;
    }
    send_status(sfd, 0);
    //创建临时文件
    remove(TMP_FILE);
    int fd = open(TMP_FILE, O_RDWR|O_CREAT, 0666);
    //将目录信息写入临时文件
    if(S_ISDIR(file_stat.st_mode)){
        dir_scan(file_name, fd);
    }else{
        file_ls(file_name, fd);
    }
    close(fd);
    //发送文件大小
    send_file_len(sfd, TMP_FILE);
    fd = open(TMP_FILE, O_RDWR);
    //发送临时文件
    send_file_by_fd(sfd, fd);
    close(fd);
    printf("send tmp file over!\n");
    return 0;
}
