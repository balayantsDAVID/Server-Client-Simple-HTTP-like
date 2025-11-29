#ifndef HTTP_H
#define HTTP_H

#define PORT 8080

#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <ctype.h>

extern pthread_mutex_t print_mutex;


typedef enum {
    HTTP_UNKNOWN = 0, 
    
    HTTP_GET,
    HTTP_POST,
    HTTP_HEAD,
    
    HTTP_ECHO,
    
    HTTP_UNSUPPORTED
} HTTP_Method;

typedef struct {
    HTTP_Method method;
    char *url;
    char *version;

    int body_size;
    char* body;
} HTTP_Request;


typedef enum {
    STATUS_OK = 200,
    STATUS_CREATED = 201,

    STATUS_BAD_REQUEST = 400,
    STATUS_NOT_FOUND = 404,
    
    STATUS_SERVER_ERROR = 500
} HttpStatus;

typedef struct {
    HttpStatus status_code;
    char *version;

    char *content_type;

    char *body;
    int content_length;

} HTTP_Response;


int server_create (int port, struct sockaddr_in *server);
int connect_to_server (int port, struct sockaddr_in *server, const char* ip);
void parse_request(char *raw_request, HTTP_Request *request);
char* get_status_text(HttpStatus status_code);
void process_request(HTTP_Request *req, HTTP_Response *resp);
void send_response(int client_fd, HTTP_Response *resp);
void* handle_client(void *arg);

#endif