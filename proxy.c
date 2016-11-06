#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>

#include <time.h>


#define MAX_BUFFER 10*1024		 //10KB buffer size for HTTP 
#define MAX_CACHE_NUM 10 	     //support 10 entry in cache

//structure to store HTTP GET Request
typedef struct _request
{
	char * host;
	char * resource;
	time_t expires;
	int visit;

	char * response;	//the response of the quest from the server
	int  len;
	struct _request * next;
}http_request;


http_request* parseHTTPPacket(char* buffer);
int processRequest(int sock, char* input_buffer, int input_len);


http_request* cache = NULL;	//the header of cache
int cache_num = 0;			//the number of request in cache

int main(int argc, char const *argv[])
{

	struct hostent *server;
	int server_sockfd, client_sockfd;
	int client_len;


	int server_port;

	int i,size;

 	fd_set readfds,activefds;
 	int fd_max;
	struct sockaddr_in server_addr, client_addr;

	char input_buffer[MAX_BUFFER];	//buffer to store packet from client



	if (argc != 3) {
		printf("Invalid parameter.\nUsage: %s <ip address> <port number>\n", argv[0]);
		exit(0);
	}

	server_port = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		perror("Can not binding to the ip address\n");
		exit(0);
	}

	bzero((char*)&server_addr, sizeof(struct sockaddr_in));

	server_addr.sin_family = AF_INET;	
  	server_addr.sin_addr.s_addr = INADDR_ANY;
  	server_addr.sin_port = htons(server_port);
  	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero); 

	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
		

 	 server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

 	 if (server_sockfd < 0)
 	 {
  	 	 perror("Error creating socket\n");
  	 	    exit(0);
  	}

	if((bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
  	{
       perror("Error binding\n");
 	   exit(0);
  	}
  

	if(listen(server_sockfd, 5) < 0)
  	{
    	perror("Error listening\n");
    	exit(0);
  	}

	FD_ZERO(&readfds);
	FD_ZERO(&activefds);

	FD_SET(server_sockfd, &activefds);

	fd_max = server_sockfd;

	while (1) {
		readfds = activefds;
      
  		if (select (fd_max + 1, &readfds, NULL, NULL, NULL) < 0){
 			 perror("Select() error!");
 			 exit(-1); 
		}

		for (i = 0; i < fd_max + 1; i++) {
			
			if (FD_ISSET(i, &readfds)) {
				if (server_sockfd == i) {
					size = sizeof(client_addr);
					client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&size);
					if (client_sockfd == -1) {
						perror("Error in connection with client\n");
						exit(0);
					}
					else {//if sucessfully accept client 
						FD_SET(client_sockfd, &activefds);
						
						if (fd_max < client_sockfd)
						{
							fd_max = client_sockfd;
						}
					}
				}
				else{// request from client

					int num = recv(i, input_buffer, MAX_BUFFER, 0);

					if (num > 0) {
						if (isGetRequest(input_buffer)) {

							processRequest(i, input_buffer, num);

							//clean up
							FD_CLR(i, &activefds);
							close(i);
						}
					}

				}
			}
		}//end of for 
	}//end of while
	close(server_sockfd);
	return 0;
}



//function: to judge if the incoming http request is "GET"
//return value: 1 if the http request is GET request; 0 if the http request is not GET request
int isGetRequest(char* http){
	if (strncmp(http, "GET", 3) == 0){
		return 1;
	}
	return 0;
}

void clearVisitFlag(){
	http_request* entry = cache;

	while(entry!= NULL){
		entry->visit = 0;
	}
}	

//function: replace LRU entry in the cache
void replaceLRURequest(http_request* request){
	http_request* least_visit;
	http_request* prev_least;

	http_request* prev;
	http_request* entry = cache;

	least_visit = cache;

	while(entry!= NULL){

		if(entry->visit < least_visit->visit){
			prev_least = prev;
			least_visit = entry;
		}
		prev = entry;
		entry = entry->next;
	}
	
	clearVisitFlag();

	//replace
	prev_least->next = request;
	request->next = least_visit->next;

}

//function: process HTTP request
int processRequest(int sock, char* input_buffer, int input_len){
	struct hostent *web_server;
	struct sockaddr_in server_addr;

	int web_sockfd;


	time_t present;
	time(&present);
	struct tm *t = gmtime(&present);

	http_request* request = parseHTTPPacket(input_buffer);

	//debug: print out the host and resource in client's request
	//perror(request->host);
	//perror(request->resource);
	//if the request hit entry in the cache
	http_request* entry = cache;
		while (entry != NULL) {
			if (!strcmp(entry->host, request->host) 
				&& !strcmp(entry->resource, request->resource)) 
			{
				/*
				time_t cur_time;
				time(&cur_time);
				struct tm *tim = gmtime(&present);
	
				printf("Cache has this item!\n");

				if (mktime(tim) > request->expires) {
					char * buffer = malloc(100 * sizeof(char));
					time(&request->expires);
					struct tm *tim = gmtime(&request->expires);

					
					perror("Page is expired!\n");

			*/

		
					//debug
					perror("Debug: Hit the cache!");

					entry->visit++;
					responseClient(sock, entry);
					return 1;

				}
			entry = entry->next;
	}//end of while
		

	//if the request cannot hit the cache
	web_server = gethostbyname(request->host);

	server_addr.sin_family = AF_INET;	
  	server_addr.sin_port = htons(80);
  	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero); 

	bcopy((char *)web_server->h_addr, (char *)&server_addr.sin_addr.s_addr, web_server->h_length);

	 web_sockfd = socket(AF_INET, SOCK_STREAM, 0); 
     if (web_sockfd < 0){
         perror("ERROR opening web socket");
         exit(0);
     }

     if (connect(web_sockfd, &server_addr, sizeof(server_addr)) == -1) {
			close(web_sockfd);
			perror("Error in connecting web server");
			exit(0);
	}

	//proxy client request to web server

	//debug
	perror("Debug: proxy request to the web server");
	if ((send(web_sockfd, input_buffer, input_len, 0)) == -1) {
		perror("Error in proxying packet to the webserver");
		exit(0);
	}

	char recv_buffer[MAX_BUFFER];
	int  recv_num;
	//handle response from reply for the web server

	if ((recv_num = recv(web_sockfd, recv_buffer, MAX_BUFFER, 0)) > 0) {
		request->response = (char *)malloc(recv_num);
		memcpy(request->response, recv_buffer, recv_num);
		request->len = recv_num;

		//parse expires
		/*
		char* expires = NULL;
		char* token = strtok(strdup(request->response), "\r\n");
			while (token != NULL) {
				if (strncmp(token, "Expires: ", 9) == 0) {
					expires = malloc(strlen(token) - 9);
					strncpy(expires, token+9, strlen(token)-9);
					break;
				}
				token = strtok(NULL, "\r\n");
		}
		if (expires != NULL && strcmp(expires,"-1") != 0){
			strptime(expires, "%a, %d %b %Y %H:%M:%S %Z", &t);
			request->expires = mktime(&t);
		}
		*/
		//debug
		//perror("expires at: " + request->expires);

		//store request into cache
		if (cache_num < MAX_CACHE_NUM){
			cache_num++;

			entry = cache;
			if (entry == NULL){
				cache = request;
			}
			else{
				while (entry->next != NULL)	
					entry = entry->next;
					entry->next = request;
			}
		}
		else{
			//LRU: remove the least visited entry
			replaceLRURequest(request);
		}
		//debug
		perror(request->response);

		responseClient(sock, request);


	}	
}

//Function： parse HTTP request from buffer
//return value: http_request
http_request* parseHTTPPacket(char* buffer){
	http_request *request = malloc(sizeof(http_request));
	int i;

	char * line = strtok(strdup(buffer), "\r\n");
	while(line) {

   		if (strncmp(line, "GET ", 4) == 0){

   			request->resource = malloc(strlen(line) - 12);
 			strncpy(request->resource, line + 4, strlen(line) - 12);
   		//	perror(request->resource);
   		}
   		if (strncmp(line, "Host: ", 5) == 0){
   			request->host = malloc(strlen(line) - 6);
   			strncpy(request->host, line + 6, strlen(line) - 6);
   			//perror(request->host);
   		}

   		line  = strtok(NULL, "\r\n");
	}

	request->expires = 0;
	request->visit = 1;

	return request;
}


//function: response to client
//return value: 1 if reponsse success
int responseClient(int sock, http_request* req){
	if ((send(sock, req->response, req->len, 0)) == -1) {
		perror("Error in reponse!");
	}
	return 1;;
}
