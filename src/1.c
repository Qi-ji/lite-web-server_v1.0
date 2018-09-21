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

char buffer[] = " GET / HTTP/1.1\r\n\
 Host: 192.168.1.141:8888\r\n\
 User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:61.0) Gecko/20100101 Firefox/61.0\r\n\
 Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
 Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\n\
 Accept-Encoding: gzip, deflate\r\n\
 Connection: keep-alive\r\n\
 Upgrade-Insecure-Requests: 1\r\n\
 Cache-Control: max-age=0\r\n\
 \r\n\
 131345646\r\n";

int line_num = 0;


/**
  *@brief  parse the message from client.
  *@param  buf - the message from client.
  *@retval the pointer to the struct 'http_message',which contains the parsed message.
  */	
http_message_t parse_buf(char *buf)
{
	const char find_balnk[] = " ";						
	const char find_colon[] = ":";						
	const char find_Enter[5] = "\r\n";					
	char *token;												
	int i = 1;
	
	char *parse_lineinfo[1024];

	http_message_t hm ;
	
	token = strtok(buf, find_Enter);									//find out the line break,and then parase the info by line first.
	while(token != NULL)
	{
		parse_lineinfo[line_num] = token;										
		line_num++;
		token = strtok(NULL, find_Enter);
	}
	
	printf("parse_lineinfo[0] = %s.\n",parse_lineinfo[0]);
	printf("parse_lineinfo[1] = %s.\n",parse_lineinfo[1]);
	// printf("parse_lineinfo[2] = %s.\n",parse_lineinfo[2]);
	/* **********************start: parse the recv into the 'http_message'*********************************/
	hm.method.p = strstr(parse_lineinfo[0], find_balnk);
	hm.method.p += strlen(find_balnk);
	hm.uri.p = strstr(hm.method.p, find_balnk);
	hm.uri.p += strlen(find_balnk);
	hm.protocol.p = strstr(hm.uri.p, find_balnk);
	hm.protocol.p += strlen(find_balnk);	


	hm.protocol.size = strlen(hm.protocol.p);	
	hm.uri.size = (strlen(hm.uri.p) - hm.protocol.size -1);
	hm.method.size = (strlen(hm.method.p) - (strlen(hm.uri.p)) -1);
		
	printf("%.*s\n",hm.method.size, hm.method.p);
	printf("%.*s\n",hm.uri.size, hm.uri.p);
	printf("%.*s\n",hm.protocol.size, hm.protocol.p);
	
	printf("strlen = %d\n", strlen(parse_lineinfo[10]));
	printf("line_num = %d\n", line_num);

	for(i=1;i<line_num;i++)
	{
		if(strlen(parse_lineinfo[i]) < 2)
		{
			if((i+1) >= line_num)
			{
				printf("123123i=%d\n", i);				
				continue;
			}
			hm.body.p = parse_lineinfo[i+1];
			break;
			
		}

		// hm.header[i].p = strstr(parse_lineinfo[i], find_balnk);
		// hm.header[i].p += strlen(find_balnk);
		// hm.header_value[i].p = strstr(hm.header[i].p, find_colon);
		// hm.header_value[i].p = strstr(hm.header[i].p, find_colon);
		// hm.header_value[i].p += strlen(find_colon);
		
		// hm.header_value[i].size = strlen(hm.header_value[i].p);
		// hm.header[i].size = strlen((hm.header[i].p - strlen(hm.header_value[i].p) -1));
		
		hm.header[i].p = strstr(parse_lineinfo[i], find_balnk);
		hm.header[i].p += strlen(find_balnk);
		hm.header_value[i].p = strstr(hm.header[i].p, find_colon);
		hm.header_value[i].p = strstr(hm.header[i].p, find_colon);
		hm.header_value[i].p += strlen(find_colon);
		
		hm.header_value[i].size = strlen(hm.header_value[i].p);
		hm.header[i].size = (strlen(hm.header[i].p)) - (strlen(hm.header_value[i].p)) -1;
		
		printf("%.*s\n",hm.header[i].size, hm.header[i].p);
		printf("%.*s\n",hm.header_value[i].size, hm.header_value[i].p);
		
	}
	printf("%s\n",hm.body.p);
	// printf("%.*s\n",hm.header[i].size, hm.header[i].p);
	// printf("%s\n", hm.header[1].p);
	// printf("%d\n", hm.header[1].size);
	// printf("%.*s\n",hm.header_value[i].size, hm.header_value[i].p);
	// printf("%.*s\n",hm.header[i].size, hm.header[i].p);
	// printf("%.*s\n",hm.header[3].size, hm.header[3].p);
	
	/* **********************end: parse the recv into the 'http_message'*********************************/
	return hm;
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
	struct http_message http_message_tocliend;
	http_message_tocliend = parse_buf(buffer);
	printf("line_num=%d\n", line_num);
	/*
	 for(i=1; i<line_num; i++)
    {
        printf("%s,%d.\n",http_message_tocliend.header[i].p, http_message_tocliend.header[i].size);
        printf("%s,%d.\n",http_message_tocliend.header_value[i].p, http_message_tocliend.header_value[i].size);
				
    }
	*/
}














	