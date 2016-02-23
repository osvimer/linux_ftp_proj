#include "identify.h"
#include "trans_file.h"

void get_identify(int sfd, char * username){
    char name[20] = {0};
    char * passwd = NULL;
    int remote_status;
    int confirm_status = 0;
    data_t buf;
    while(!confirm_status){
        system("clear");
        printf("****************** FTP SYSTEM ******************\n");
        printf("[Sign in]\n");
        //获取并发送用户名
        printf("username: ");
        bzero(name, sizeof(name));
        scanf("%s", name);
        bzero(&buf, sizeof(buf));
        strcpy(buf.data, name);
        buf.len = strlen(buf.data);
        send_complete(sfd, &buf);
        recv_status(sfd, &remote_status);
        if(-1 == remote_status){
            printf("username error!\n");
            continue;
        }
        //获取并发送密码
        passwd = getpass("password: ");
        bzero(&buf, sizeof(buf));
        strcpy(buf.data, passwd);
        buf.len = strlen(buf.data);
        send_complete(sfd, &buf);
        recv_status(sfd, &remote_status);
        if(-1 == remote_status){
            printf("password error!\n");
            continue;
        }
        confirm_status = 1;
        strcpy(username, name);
        printf("login sucess!\n");
    }
}
