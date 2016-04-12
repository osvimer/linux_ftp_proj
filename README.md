* Author         : Junjie Huang
* Email          : acmhjj@gmail.com
* Description    : FTP Server / Client
* version        : 0.03
- 初步功能：密码验证, 守护进程，日志记录(完成)
- 二期功能：断点续传，内存映射(大文件下载/上传)(完成)
- 三期功能：多线程下载，实现数据、命令双通道
- TODO：遵循FTP协议(RFC 959)，并实现其他功能

#### 使用说明：

1. git clone https://github.com/osvimer/linux_ftp_proj.git

2. cd linux_ftp_proj

3. make

4. 修改服务端配置文件（IP、端口、根目录、最大连接数）
    > $ vim server/conf/server.conf

5. 启动服务端
    > $ sudo server/bin/server server/conf/server.conf

6. 启动客户端 
    > $ client/bin/client $SERVER_IP $SERVER_PORT

7. 以服务端所在系统中实际存在的用户名密码登录

8. 输入 help 命令查看帮助
