### ABOUT：

* Author         : Junjie Huang
* Email          : acmhjj@gmail.com
* Description    : FTP Server / Client
* version        : 0.03
- Phase I：user identify, set daemon，log records. (completed)
- Phase II：resuming，mmap (large files download/upload). (completed)
- Phase III：multi-thread download / upload，dual-channel transport (data and command).
- TODO：support the FTP Protocol in the RFC 959, add some other functions.

### HOWTO：

1. git clone https://github.com/osvimer/linux_ftp_proj.git

2. cd linux_ftp_proj

3. make

4. Edit the server configuration files to configure the FTP server (IP, Port, Root Directory, Max peers and so on).
    > $ vim server/conf/server.conf

5. Start the FTP Server
    > $ sudo server/bin/server server/conf/server.conf

6. Start the FTP Client
    > $ client/bin/client SERVER_IP SERVER_PORT

7. Login. Use the real username and password of the system in which the FTP server runs.

8. Type 'help' to learn how to use the client system.

### CHANGELOG:：

    Version 0.3.0 - 12 Apr 2016:
        - Add some useful interaction.
        - Minor changes

    Version 0.2.0 - 25 Feb 2016:
        - Add resuming and mmap

    Version 0.1.0 - 18 Feb 2016:
        - Implemented list command
        - Minor changes
