#include "init.h"
#include "service.h"
#include "trans_fd.h"
#include "process_poll.h"

int server_status = 1;

int main (int argc, char const* argv[]){
    if(argc != 2){
        printf("./server ../conf/server.conf\n");
        return -1;
    }
    //设置服务端为守护进程
    set_daemon();
    //读取配置文件
    int conf_fd = open(argv[1], O_RDONLY);
    if(-1 == conf_fd){
        perror("open config");
        return -1;
    }
    char buf[512];
    bzero(buf, sizeof(buf));
    int ret = read(conf_fd, buf, sizeof(buf));
    if(-1 == ret){
        perror("read config");
        return -1;
    }
    //获取配置参数
    struct server_config s_conf;
    bzero(&s_conf, sizeof(s_conf));
    sscanf(buf, "%*[^=]=%s%*[^=]=%d%*[^=]=%d%*[^=]=%d%*[^=]=%s%*[^=]=%s", s_conf.ip, &s_conf.port, &s_conf.peer_max, &s_conf.log_enable, s_conf.log_file, s_conf.root_dir);
    if(!(* s_conf.ip) || !s_conf.port){
        printf("config error!");
        return -1;
    }
    //最大连接数
    int peer_max = s_conf.peer_max;
    if(peer_max <= 0){
        peer_max = 10;//获取失败或配置错误，置为默认值
    }
    //切换到ftp根目录
    ret = chdir(s_conf.root_dir);
    if(-1 == ret){
        perror("chdir");
        return -1;
    }
    //打开日志记录文件,开始记录日志
    int log_fd;
    if(s_conf.log_enable){
        log_fd = open(s_conf.log_file, O_WRONLY|O_CREAT, 0666);
        if(-1 == log_fd){
            perror("open log file");
            return -1;
        }
    }
    //创建进程池
    pchild_t cptr;
    cptr = (pchild_t)calloc(peer_max, sizeof(child_t));
    make_child(cptr, peer_max);
    //关闭配置文件
    close(conf_fd);
    //建立SOCKET连接
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfd){
        perror("socket");
        return -1;
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(s_conf.ip);
    server_addr.sin_port = htons(s_conf.port);
    ret = bind(sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if(-1 == ret){
        perror("bind");
        return -1;
    }
    ret = listen(sfd, peer_max);
    if(-1 == ret){
        perror("listen");
        return -1;
    }
    //创建EPOLL句柄并监听SOCKET
    int epfd = epoll_create(1);
    if(-1 == epfd){
        perror("epoll_create");
        return -1;
    }
    struct epoll_event epev, epevs[peer_max + 1];
    epev.events = EPOLLIN;
    epev.data.fd = sfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &epev);
    if(-1 == ret){
        perror("epoll_ctl1");
        return -1;
    }
    //监听子进程通信fd
    int i, j;
    for(i = 0; i < peer_max; i++){
        epev.events = EPOLLIN;
        epev.data.fd = cptr[i].pfd;
        ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cptr[i].pfd, &epev);
        if(-1 == ret){
            perror("epoll_ctl2");
            return -1;
        }
    }
    int pmsg = 1;
    int newfd = -1;
    //开始任务处理(监听SOCKET连接，分配任务给子进程，记录子进程状态)
    while(server_status){
        bzero(epevs, sizeof(epevs));
        ret = epoll_wait(epfd, epevs, peer_max + 1, -1);
        if(ret > 0){
            for(i = 0; i < ret; i++){
                if(epevs[i].data.fd == sfd && epevs[i].events == EPOLLIN){
                    newfd = accept(sfd, NULL, NULL);
                    if(-1 == newfd){
                        perror("accept");
                        return -1;
                    }
                    printf("accept a new client!\n");
                    for(j = 0; j < peer_max; j++){
                        if(0 == cptr[j].busy){
                            break;
                        }
                    }
                    if(j < peer_max){
                        //将newfd交付给子线程，并表示为忙碌
                        send_fd(cptr[j].pfd, newfd);
                        cptr[j].busy = 1;
                    }
                }
                for(j = 0; j < peer_max; j++){
                    if(epevs[i].data.fd == cptr[j].pfd && epevs[i].events == EPOLLIN){
                        read(cptr[j].pfd, &pmsg, sizeof(int));
                        cptr[j].busy = 0;
                    }
                }
            }
        }
    }
    close(epfd);
    close(sfd);
    close(log_fd);
    return 0;
}
