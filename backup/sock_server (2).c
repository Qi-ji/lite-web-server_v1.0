    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sys/time.h>
 
    #define DEFAULT_PORT    9003    //默认端口
    #define BUFF_SIZE       1024    //buffer大小
    #define SELECT_TIMEOUT  5       //select的timeout seconds
	#define SERADDR		"192.168.1.141"
 
    //函数：设置sock为non-blocking mode
    void setSockNonBlock(int sock) {
        int flags;
        flags = fcntl(sock, F_GETFL, 0); 					 //获得block原来的属性
        if (flags < 0) {
            perror("fcntl(F_GETFL) failed");
            exit(EXIT_FAILURE);
        }
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
            perror("fcntl(F_SETFL) failed");
            exit(EXIT_FAILURE);
        }
    }
    //函数：更新maxfd
    int updateMaxfd(fd_set fds, int maxfd) {
        int i;
        int new_maxfd = 0;
        for (i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &fds) && i > new_maxfd) {		//检查i是否在fds这个集合里面
                new_maxfd = i;
            }
        }
        return new_maxfd;
    }
 
    int main(int argc, char *argv[]) {
        unsigned short int port;
 
        //获取自定义端口，可以通过参数输入需要连接的端口号***********************************
        if (argc == 2) {
            port = atoi(argv[1]);
        } else if (argc < 2) {
            port = DEFAULT_PORT;
        } else {
            fprintf(stderr, "USAGE: %s [port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
 
        //创建socket*************************************************************************
        int sock;
		/*理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。当然AF_INET和PF_INET的值是相同的，混用也不会有太大的问题。*/
        if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) == -1 ) {		
            perror("socket failed, ");
            exit(EXIT_FAILURE);
        }
        printf("socket done\n");
 
        //防止出现 'address already in use' error message************************************
        int yes = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }
 
        //设置sock为non-blocking
        setSockNonBlock(sock);
 
        //创建要bind的socket address*********************************************************
        struct sockaddr_in bind_addr;
        memset(&bind_addr, 0, sizeof(bind_addr));
        bind_addr.sin_family = AF_INET;
        //bind_addr.sin_addr.s_addr = htonl(SERADDR);  //设置接受任意地址
		bind_addr.sin_addr.s_addr = inet_addr(SERADDR);
        bind_addr.sin_port = htons(port);               //将host byte order转换为network byte order
 
        //bind sock到创建的socket address上****************************************************
        if ( bind(sock, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) == -1 ) {
            perror("bind failed, ");
            exit(EXIT_FAILURE);
        }
        printf("bind done\n");
 
        //listen*************************************************设置监听队列最多为5个*********
        if ( listen(sock, 5) == -1) {
            perror("listen failed.");
            exit(EXIT_FAILURE);
        }
        printf("listen done\n");
 
        //创建并初始化select需要的参数(这里仅监视read)，并把sock添加到fd_set中*****************
        fd_set readfds;
        fd_set readfds_bak; //backup for readfds(由于每次select之后会更新readfds，因此需要backup)
        struct timeval timeout;
        int maxfd;
        maxfd = sock;
        FD_ZERO(&readfds);					//将指定的文件描述符集清空
        FD_ZERO(&readfds_bak);
        FD_SET(sock, &readfds_bak);			//用于在文件描述符集合中增加一个新的文件描述符
 
        //循环接受client请求
        int new_sock;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;
        char client_ip_str[INET_ADDRSTRLEN];
        int res;
        int i;
        char buffer[BUFF_SIZE];
		//const char *str = "Maize,it is time to eat lunch!";
        int recv_size;
 
        while (1) {
 
            //注意select之后readfds和timeout的值都会被修改，因此每次都进行重置
            readfds = readfds_bak;
            maxfd = updateMaxfd(readfds, maxfd);        //更新maxfd
            timeout.tv_sec = SELECT_TIMEOUT;
            timeout.tv_usec = 0;
            printf("selecting maxfd=%d\n", maxfd);
 
            //select(这里没有设置writefds和errorfds，如有需要可以设置)
            res = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
            if (res == -1) {
                perror("select failed");
                exit(EXIT_FAILURE);
            } else if (res == 0) {
                fprintf(stderr, "no socket ready for read within %d secs\n", SELECT_TIMEOUT);
                continue;
            }
 
            //检查每个socket，并进行读(如果是sock则accept)
            for (i = 0; i <= maxfd; i++) {
                if (!FD_ISSET(i, &readfds)) {
                    continue;
                }
                //可读的socket
                if ( i == sock) {
                    //当前是server的socket，不进行读写而是accept新连接
                    client_addr_len = sizeof(client_addr);
                    new_sock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
                    if (new_sock == -1) {
                        perror("accept failed");
                        exit(EXIT_FAILURE);
                    }
                    if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
                        perror("inet_ntop failed");
                        exit(EXIT_FAILURE);
                    }
                    printf("accept a client from: %s\n", client_ip_str);
                    //设置new_sock为non-blocking
                    setSockNonBlock(new_sock);
                    //把new_sock添加到select的侦听中
                    if (new_sock > maxfd) {
                        maxfd = new_sock;
                    }
                    FD_SET(new_sock, &readfds_bak);
                } else {
                    //当前是client连接的socket，可以写(read from client)
                    memset(buffer, 0, sizeof(buffer));
                    if ( (recv_size = recv(i, buffer, sizeof(buffer), 0)) == -1 ) {
                        perror("recv failed");
                        exit(EXIT_FAILURE);
                    }
                    printf("recved from new_sock=%d : %s(%d length string)\n", i, buffer, recv_size);
                    //立即将收到的内容写回去，并关闭连接
					memset(buffer, 0, sizeof(buffer));
					//if ( send(i, str, strlen(str), 0) == -1 ) {
                    if ( send(i, buffer, recv_size, 0) == -1 ) {
                        perror("send failed");
                        exit(EXIT_FAILURE);
                    }
                    printf("send to new_sock=%d done\n", i);
                    if ( close(i) == -1 ) {
                        perror("close failed");
                        exit(EXIT_FAILURE);
                    }
                    printf("close new_sock=%d done\n", i);
                    //将当前的socket从select的侦听中移除
                    FD_CLR(i, &readfds_bak);
                }
            }
        }
 
        return 0;
    }