#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <assert.h>

#define MAX_BUFFER 10*1024       //10KB buffer size for HTTP 

int main(int argc,char *argv[])
{
    int sockfd;
    int port;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char send_buffer[1500];
    char url[1000];
  
    char recv_buff[512];

    char host[100];
    char uri[100];

    //for parse url
    int i=0;
    int j=0;
    int k=0;

    int readbytes = 0;

    if (argc != 4)
    {   perror("Invalid parameter.\nUsage: ./client PROXY_SERVER_IP PROXY_PORT URL\n");
        exit(0);
    }


    port = atoi(argv[2]);
    strcpy(url,argv[3]);
    server = gethostbyname(argv[1]);

    if (server ==NULL)
    {
        perror("Error in locating server");
        exit(0);
    }

    //parsing host and uri from the url
    while(url[i] != '/') {       
        host[j++] = url[i++];
    }



    while(i < strlen(url)){
         uri[k++] = url[i++];
    }
    uri[0] = '/';
    url[k] = '\0';

    //create socket 
    if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0)
    {        
        printf( "Cannot create Socket\n");
        exit(1);
    }


    printf("Host: %s\n", host);
    printf("URI: %s\n", uri);

    bzero((char *)&server_addr, sizeof(server_addr));

    bcopy ((char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
            printf("ERROR CONNECTING\n");
            exit(1);

    }

    sprintf(send_buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: HTMLGET 1.0\r\n\r\n",uri,host);
    printf("%s\n",send_buffer);

    if (send(sockfd ,send_buffer,sizeof(send_buffer),0) == -1)
    {
        perror("Error in sending request.\n");
        exit(0);
    }

    //output response
     while((readbytes = recv(sockfd, recv_buff, MAX_BUFFER, 0)) > 0)
    {
        printf("%s", recv_buff);
        fflush(stdout);
    }
   

    close(sockfd);
    return(1);

}
