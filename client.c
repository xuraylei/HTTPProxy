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

#define BUFFER_SIZE 2048000
#define CACHE_SIZE 10

typedef struct data_block {
char message[BUFFER_SIZE];
int size;
} Data_block;


int main(int argc,char *argv[])
{
int sockfd,connectfd,port,readbytes=1;
struct sockaddr_in saddress;
struct hostent *server;
char message_buff[1500];
char URL[1000];
char domain_name[500];
char page[500];
char recv_buff[512];
FILE *f1;
if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0)
{        printf( "Cannot create Socket\n");
        exit(1);
}

if (argc != 4)
{        printf(" usage: ./client PROXY_SERVER_IP PROXY_PORT URL\n");
        exit(1);
}
 else {
        port = atoi(argv[2]);
        strcpy(URL,argv[3]);
        server = gethostbyname(argv[1]);
        }
if (server ==NULL)
 {printf("no such host\n");
 exit(1);
}

printf("URL is  %s \n", &URL);

int i=0;
int j=0;
int k=0;
while(URL[i] != '/' && URL[i+1] != '/')
        {        i++;
        }
i=i+3;

while ( URL[i] != '/')
         {   domain_name[k++] = URL[i++];
         }
i++;
while (i < strlen(URL) )
         {    page[j++] = URL[i++];
         }
printf("domain name: %s\n",domain_name);
printf("page: %s\n",page);

bzero((char *)&saddress, sizeof(saddress));

bcopy ((char *)server->h_addr,(char *)&saddress.sin_addr.s_addr,server->h_length);
saddress.sin_port = htons(port);
saddress.sin_family = AF_INET;
if ((connectfd = connect(sockfd, (struct sockaddr*) &saddress, sizeof(saddress))) < 0) {
        printf("ERROR CONNECTING\n");
        exit(1);

}

sprintf(message_buff, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: HTMLGET 1.0\r\n\r\n",page,domain_name);

printf("%s\n",message_buff);

if (send(sockfd ,message_buff,sizeof(message_buff),0) == -1)
              {
                 fprintf(stderr, "error in sending\n");
                 exit(1);
              }


f1=fopen("recvfile.txt","a");

        while(readbytes!=0)
        {
                readbytes = recv(sockfd, recv_buff, 512, 0);
                fprintf(f1,"%s",recv_buff);
        }

fclose(f1);

close(sockfd);
return(1);

}
