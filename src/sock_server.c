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

#define DEFAULT_PORT    8888    					//the default port
#define BUFF_SIZE       1024	   					//the size of buffer
#define SELECT_TIMEOUT  5       					//timeout seconds for 'select'
#define SERADDR		"192.168.1.141"
#define PRASE_MAX_LINE_NUM  20

struct http_str 
{
	char *p;
	int size;
};

typedef struct http_message 
{
	int status_code;
	struct http_str method;
	struct http_str uri;
	struct http_str uri_query;
	struct http_str protocol;
	struct http_str header[40];
	struct http_str header_value[40];
	struct http_str body;
}http_message_t;

int line_num = 0;

/**
  *@brief  set the socket in non-blocking mode
  *@param  sock --the socket descriptor
  *@retval none
  */
  /*
void setSockNonBlock(int sock) 
{
    int flags;
    flags = fcntl(sock, F_GETFL, 0); 					 
    if (flags < 0) {
        perror("fcntl(F_GETFL) failed");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl(F_SETFL) failed");
        exit(EXIT_FAILURE);
    }
}
*/
/**
  *@brief  socket、bind and listen to client
  *@param  none 
  *@retval the socket descriptor
  */	
int sock_bind_listen (void)
{
    int sock;
	/*理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。\
	当然AF_INET和PF_INET的值是相同的，混用也不会有太大的问题。*/
    if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) == -1 ) {					//socket
        perror("socket failed, ");
        exit(EXIT_FAILURE);
    }
    printf("socket done\n");

    /***start:prevent the errot: 'address already in use' error message***/
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
	 /***end:prevent the errot: 'address already in use' error message***/
    /*set the sock as non-blocking.*/
    // setSockNonBlock(sock);

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    //bind_addr.sin_addr.s_addr = htonl(SERADDR);  							//设置接受任意地址
	bind_addr.sin_addr.s_addr = inet_addr(SERADDR);
    bind_addr.sin_port = htons(DEFAULT_PORT);               				//change 'host byte order'as 'network byte order'

    /****************************start:bind *******************/
    if ( bind(sock, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) == -1 ) {
        perror("bind failed, ");
        exit(EXIT_FAILURE);
    }
    printf("bind done\n");
    /****************************end:bind *********************/
    /****************************start:listen *****************/
    if ( listen(sock, 5) == -1) {
        perror("listen failed.");
        exit(EXIT_FAILURE);
    }
    printf("listen done\n");
	/****************************end:bind *********************/
	return sock;
}
	
/**
  *@brief  update the maxfd
  *@param  fds--the fd_set type variable
  *		   maxfd--the max fd
  *@retval the max fd
  */
int updateMaxfd(fd_set fds, int maxfd)
{
    int i;
    int new_maxfd = 0;
    for (i = 0; i <= maxfd; i++) 
	{
        if (FD_ISSET(i, &fds) && i > new_maxfd)
		{		//检查i是否在fds这个集合里面
            new_maxfd = i;
		}
    }
        return new_maxfd;
    }

/**
  *@brief  parse the message from client.
  *@param  buf - the message from client.
  *@retval the pointer to the struct 'http_message',which contains the parsed message.
  */	
http_message_t parse_buf(char *buf)
{
	const char find_balnk[2] = " ";						
	const char find_Enter[5] = "\r\n";					
	char *token;												
	int i = 1;
	
	char *parse_lineinfo[PRASE_MAX_LINE_NUM];

	http_message_t hm ;
	
	token = strtok(buf, find_Enter);									//find out the line break,and then parase the info by line first.
	while(token != NULL)
	{
		parse_lineinfo[line_num] = token;										
		line_num++;
		token = strtok(NULL, find_Enter);
	}
	
	/* **********************start: parse the recv into the 'http_message'*********************************/
	hm.method.p = strstr(parse_lineinfo[0], find_balnk);
	hm.method.p += strlen(find_balnk);
	hm.uri.p = strstr(hm.method.p, find_balnk);
	hm.uri.p += strlen(find_balnk);
	hm.protocol.p = strstr(hm.method.p, find_balnk);
	hm.protocol.p += strlen(find_balnk);	

	hm.protocol.size = strlen(hm.protocol.p);	
	hm.uri.size = (strlen(hm.uri.p) - hm.protocol.size -1);
	hm.method.size = (strlen(hm.method.p) - (strlen(hm.uri.p)) -1);
		
	for(i=1;i<line_num;i++)
	{
		if(strlen(parse_lineinfo[i]) == 0)
		{
			hm.body.p = parse_lineinfo[i+1];
			
		}
		
		hm.header[i].p = strstr(parse_lineinfo[i], find_balnk);
		hm.header[i].p += strlen(find_balnk);
		hm.header_value[i].p = strstr(hm.method.p, find_balnk);
		hm.header_value[i].p += strlen(find_balnk);
		
		hm.header_value[i].size = strlen(hm.header_value[i].p);
		hm.header[i].size = strlen((hm.header[i].p - strlen(hm.header_value[i].p) -1));
	}
	/* **********************end: parse the recv into the 'http_message'*********************************/
	return hm;
}	

/**
  *@brief  send the parased buffer back to client.
  *@param  sock_fd - the file descriptor created by 'accept'.
  *		   http_message_t - the pointer to the struct 'http_message',which contains the parsed message. 
  *@retval none
  */
void send_message (int sock_fd,  http_message_t hm_pa)	
{
	int ret,i,a = 5;
	
    ret = send(sock_fd, hm_pa.method.p, hm_pa.method.size, 0); 		
    ret = send(sock_fd, hm_pa.uri.p, hm_pa.uri.size, 0); 			
    ret = send(sock_fd, hm_pa.protocol.p, hm_pa.protocol.size, 0); 	
    ret = send(sock_fd, hm_pa.protocol.p, hm_pa.protocol.size, 0); 	
	
    for(i=1; i<line_num; i++)
    {
        ret = send(sock_fd, hm_pa.header[i].p, hm_pa.header[i].size, 0); 			
        ret = send(sock_fd, hm_pa.header_value[i].p, hm_pa.header_value[i].size, 0); 			
    }

	ret = send(sock_fd, hm_pa.body.p, hm_pa.method.size, 0); 
	if ( ret == -1 ) 	//send the parased buffer back to client.
	{
        perror("send failed");
        exit(EXIT_FAILURE);
    }
    printf("send to client_sock_fd=%d done\n", sock_fd);
}
	
int main(int argc, char *argv[]) {
	
	int sock_fd;
	

	
    int client_sock_fd;
    struct sockaddr_in client_addr;
    int client_addr_len;
	
    char client_ip_str[20];
	char buffer[BUFF_SIZE];
    int recv_len,res, i;	


	http_message_t  hm_toclient ;	
	
	fd_set readfds, readfds_bak;							//backup for readfds
    struct timeval timeout;
    int maxfd; 												//the maxed fd 
	/*socket、bind and listen to client*/
	sock_fd = sock_bind_listen();							
	
    maxfd = sock_fd;
    FD_ZERO(&readfds);										//clear the specified file descriptor set
    FD_SET(sock_fd, &readfds);								//add a new file descriptoy in the set

    while (1) 
	{
		readfds_bak = readfds;								//reload the readfds
        maxfd = updateMaxfd(readfds, maxfd);   				//update maxfd, because it has been changed when accept a client.
        timeout.tv_sec = SELECT_TIMEOUT;
        timeout.tv_usec = 0;
        printf("selecting maxfd=%d\n", maxfd);

        res = select(maxfd + 1, &readfds_bak, NULL, NULL, &timeout);
        if (res == -1) 
		{
            perror("select failed");
            exit(EXIT_FAILURE);
        } 
		else if (res == 0) 
		{
            fprintf(stderr, "no socket ready for read within %d secs\n", SELECT_TIMEOUT);
            continue;
        }
															//loop to check every fd in the fd_set
        for (i=0; i<maxfd+1; i++) 
		{
            if (!FD_ISSET(i, &readfds_bak)) 
			{
                continue;
            }
													
            if (i == sock_fd) 								//i == sock_fd means there is no cilent accepat, we wait for a new accept
			{
                client_addr_len = sizeof(client_addr);		
                client_sock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_len);
                if (client_sock_fd == -1) 
				{
                    perror("accept failed");
                   // exit(EXIT_FAILURE);
                }
                if (!inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str))) {
                    perror("inet_ntop failed");
                   // exit(EXIT_FAILURE);
                }
                printf("accept a client from: %s\n", client_ip_str);
				
                // setSockNonBlock(client_sock_fd);			//set 'client_sock_fd' as 'non-blocking'	
                FD_SET(client_sock_fd, &readfds_bak);		//add 'client_sock_fd'int select's listen(fd_set).	
				
                if (client_sock_fd > maxfd) 				//update the maxfd(add the new sockfd 'client_sock_fd' from 'accept') 
				{
                    maxfd = client_sock_fd;
                }
            } 
			else 											//'i'is equal the 'client_sock_fd', then recv from client.
			{
                memset(buffer, 0, sizeof(buffer));
                if ( (recv_len = recv(i, buffer, sizeof(buffer), 0)) == -1 )
				{
                    perror("recv failed");
                   // exit(EXIT_FAILURE);
                }
                printf("recved from client_sock_fd=%d :\n%s(%d length string)\n", i, buffer, recv_len);
				
				/*parase the buffer from client.*/
				hm_toclient = parse_buf(buffer);	
				/*send message after parase to client.*/
				send_message(i, hm_toclient);		
                
				if ( close(i) == -1 ) 						//close this 'client_sock_fd'
				{
                    perror("close failed");
                    //exit(EXIT_FAILURE);
                }
                printf("close client_sock_fd=%d done\n", i);
															//remove 'client_sock_fd' from the fd_set.
                FD_CLR(i, &readfds_bak);
            }
        }
    }
    return 0;
}



	
/**
  *@brief  解析客户端发来的信息，将其信息按行解析出来
  *@param  收到的客户端的信息
  *@retval 无
  */
/*  
void parse_buf(char *buf)
{
	const char Find_balnk[2] = " ";						
	const char Find_Enter[5] = "\r\n";					
	char *token;												
	//char parse_line[20][100] = {0};				//放置按行解析后每行的信息。
	int i = 1,j = 1;
	
	typedef struct parse_line_t
	{
		int line_num;
		char *info_line;
	}parse_line;
	
//	parse_line parse_lineinfo[PRASE_MAX_LINE_NUM];			
	
	
	token = strtok(buf, Find_Enter);		//查找buf中的换行符，先将获得的信息按行解析
	while(token != NULL)
	{
//		parse_lineinfo[i].info_line = token;		//将获得的第一行放到结构体数组中

		i++;
		token = strtok(NULL, Find_Enter);
	}
	for(j=1; j<i;j++)
	{
	printf("第%d行：%s\n\n", j, parse_lineinfo[j].info_line);
	}	
		
	}	
*/


























 
	
	

 