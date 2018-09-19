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

struct http_str 
{
	char *p;
	int size;
};

struct http_message 
{
	int status_code;
	struct http_str method;
	struct http_str uri;
	struct http_str uri_query;
	struct http_str protocol;
	struct http_str header[40];
	struct http_str header_value[40];
	struct http_str body;
};

char *buffer = " GET / HTTP/1.1\
 Host: 192.168.1.141:9003\
 User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:61.0) Gecko/20100101 Firefox/61.0\
 Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/;q=0.8\
 Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\
 Accept-Encoding: gzip, deflate\
 Connection: keep-alive\
 Upgrade-Insecure-Requests: 1\
 Cache-Control: max-age=0\
";

int line_num = 0;


/**
  *@brief  parse the message from client.
  *@param  buf - the message from client.
  *@retval the pointer to the struct 'http_message',which contains the parsed message.
  */	
struct http_message *parse_buf(char *buf)
{
	const char find_balnk[2] = " ";						
	const char find_Enter[5] = "\r\n";					
	char *token;												
	int i = 1;
	
	char *parse_lineinfo[32];

	struct http_message  *http_message_t;
	
	token = strtok(buf, find_Enter);									//find out the line break,and then parase the info by line first.
	while(token != NULL)
	{
		parse_lineinfo[line_num] = token;										
		line_num++;
		token = strtok(NULL, find_Enter);
	}
	
	/* **********************start: parse the recv into the 'http_message'*********************************/
	http_message_t->method.p = strstr(parse_lineinfo[0], find_balnk);
	http_message_t->method.p += strlen(find_balnk);
	http_message_t->uri.p = strstr(http_message_t->method.p, find_balnk);
	http_message_t->uri.p += strlen(find_balnk);
	http_message_t->protocol.p = strstr(http_message_t->method.p, find_balnk);
	http_message_t->protocol.p += strlen(find_balnk);	

	http_message_t->protocol.size = strlen(http_message_t->protocol.p);	
	http_message_t->uri.size = (strlen(http_message_t->uri.p) - http_message_t->protocol.size -1);
	http_message_t->method.size = (strlen(http_message_t->method.p) - (strlen(http_message_t->uri.p)) -1);
		
	for(i=1;i<line_num;i++)
	{
		if(strlen(parse_lineinfo[i]) == 0)
		{
			http_message_t->body.p = parse_lineinfo[i+1];
		}
		
		http_message_t->header[i].p = strstr(parse_lineinfo[i], find_balnk);
		http_message_t->header[i].p += strlen(find_balnk);
		http_message_t->header_value[i].p = strstr(http_message_t->method.p, find_balnk);
		http_message_t->header_value[i].p += strlen(find_balnk);
		
		http_message_t->header_value[i].size = strlen(http_message_t->header_value[i].p);
		http_message_t->header[i].size = strlen((http_message_t->header[i].p - strlen(http_message_t->header_value[i].p) -1));
	}
	/* **********************end: parse the recv into the 'http_message'*********************************/
	return http_message_t;
}	

/**
  *@brief  send the parased buffer back to client.
  *@param  sock_fd - the file descriptor created by 'accept'.
  *		   http_message_t - the pointer to the struct 'http_message',which contains the parsed message. 
  *@retval none
  */
  /*
void send_message (int sock_fd, struct http_message *http_message_t)	
{
	int ret,i;
	
    ret = send(sock_fd, http_message_t->method.p, http_message_t->method.size, 0); 		
    ret = send(sock_fd, http_message_t->uri.p, http_message_t->uri.size, 0); 			
    ret = send(sock_fd, http_message_t->protocol.p, http_message_t->protocol.size, 0); 	
	
    for(i=1; i<line_num; i++)
    {
        ret = send(sock_fd, http_message_t->header[i].p, http_message_t->header[i].size, 0); 			
        ret = send(sock_fd, http_message_t->header_value[i].p, http_message_t->header_value[i].size, 0); 			
    }
	
	ret = send(sock_fd, http_message_t->body.p, http_message_t->method.size, 0); 
	if ( ret == -1 ) 	//send the parased buffer back to client.
	{
        perror("send failed");
        exit(EXIT_FAILURE);
    }
    printf("send to client_sock_fd=%d done\n", sock_fd);
}
*/

void main()
{
	int i = 1;
	struct http_message *http_message_tocliend;
	http_message_tocliend = parse_buf(buffer);
	 for(i=1; i<line_num; i++)
    {
        printf("%s,%d.\n",http_message_tocliend->header[i].p, http_message_tocliend->header[i].size);
        printf("%s,%d.\n",http_message_tocliend->header_value[i].p, http_message_tocliend->header_value[i].size);
				
    }

}














	