#include "identify.h"
#include "trans_file.h"
#include "init.h"

void get_salt(char * salt, char * passwd){
    int i, j;
    //取出salt,i记录密码字符下标,j记录$出现次数
    for(i = 0, j = 0; passwd[i] && j != 3; ++i){
        if(passwd[i] == '$'){
            ++j;
        }
    }
    strncpy(salt, passwd, i - 1);
}

void client_identify(int sfd){
    int confirm_status = 0;
    char username[20] = {0};
    char passwd[20] = {0};
    char salt[512] = {0};
    struct spwd * sp;
    struct passwd * pw;
    data_t buf;
    while(!confirm_status){//接收用户名密码并进行验证
        //设置有效用户ID为root,有效组ID为root
        pw = getpwnam("root");
        setegid(pw->pw_gid);
        seteuid(pw->pw_uid);
        //printf("before idendity, uid = %d, euid = %d, gid = %d, egid = %d\n", getuid(), geteuid(), getgid(), getegid());
        //接收用户名
        bzero(&username, sizeof(username));
        bzero(&buf, sizeof(buf));
        recv_complete(sfd, &buf);
        strcpy(username, buf.data);
        if((sp = getspnam(username)) == NULL){//用户名检查
            send_status(sfd, -1);
            continue;
        }else{
            //接收密码
            send_status(sfd, 0);
            bzero(&buf, sizeof(buf));
            recv_complete(sfd, &buf);
            strcpy(passwd, buf.data);
            //得到salt,用得到的密码作参数
            get_salt(salt, sp->sp_pwdp);
            bzero(&buf, sizeof(buf));
            //进行密码验证
            if(strcmp(sp->sp_pwdp, crypt(passwd, salt)) == 0){//验证通过
                send_status(sfd, 0);
                confirm_status = 1;
                //设置有效用户ID为登录用户ID,有效用户组ID为登录用户组ID
                pw = getpwnam(username);
                setegid(pw->pw_gid);
                seteuid(pw->pw_uid);
                //printf("after idendity, uid = %d, euid = %d, gid = %d, egid = %d\n", getuid(), geteuid(), getgid(), getegid());
                log_time();
                printf("%s login success\n", username);
            }else{//验证失败
                send_status(sfd, -1);
                continue;
            }
        }
    }
}
