#include "http.h"

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;


int server_create (int port, struct sockaddr_in *server){
    memset(server, 0, sizeof(*server));

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr.s_addr = INADDR_ANY;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("Socket error\n");
        return(-1);
    }

    if (bind(fd, (struct sockaddr*)&(*server), sizeof(*server)) == -1)
    {
        perror("Bind error\n");
        return(-1);
    }
    
    if (listen(fd, SOMAXCONN) == -1)
    {
        perror("Listen error\n");
        return(-1);
    }

    printf("Server created!\n");
    
    return fd;
}

int connect_to_server (int port, struct sockaddr_in *server, const char* ip){
    memset(server, 0, sizeof(*server));

    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    if(inet_pton(AF_INET, ip, &server->sin_addr.s_addr) <= 0)
    {
        perror("Inet_pton error\n");
        return(-1);
    }


    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("Socket error\n");
        return(-1);
    }

    if (connect(fd, (struct sockaddr*)&(*server), sizeof(*server)) == -1)
    {
        perror("Connect error\n");
        return(-1);
    }
    
    printf("Connected to server\n");

    return fd;
}

void parse_request(char *raw_request, HTTP_Request *request){
    char *method_end = strchr(raw_request, ' ');
    
    if (strncmp(raw_request, "GET",  method_end - raw_request) == 0)
    {
        request->method = HTTP_GET;
    }
    else if (strncmp(raw_request, "POST", method_end - raw_request) == 0)
    {
        request->method = HTTP_POST;
    }
    else if (strncmp(raw_request, "ECHO", method_end - raw_request) == 0)
    {
        request->method = HTTP_ECHO;
    }
    else
    {
        request->method = HTTP_UNKNOWN;
    }


    char *path_start = method_end + 1;
    char *path_end = strchr(path_start, ' ');
    
    request->url = strndup(path_start, path_end - path_start);

    if (!request->url)
    {
        request->method = HTTP_UNKNOWN;
        return;
    }

    char *version_start = path_end + 1;
    char *version_end = strchr(version_start, '\r');
    
    if (!version_end)
    {
        version_end = strchr(version_start, '\n');
    }
    request->version = strndup(version_start, version_end ? version_end - version_start : strlen(version_start));

    if (strcmp(request->version, "CHLP/1.0") != 0) {
        request->method = HTTP_UNKNOWN;
    }

    request->body_size = 0;
    request->body = NULL;

    char *header_start = version_end ? version_end + (version_end[0] == '\r' ? 2 : 1) : version_start + strlen(version_start) + 1;

    while (*header_start) {
        char *line_end = strstr(header_start, "\r\n");
        if (!line_end) line_end = strchr(header_start, '\n');
        if (!line_end) break;
        if (line_end - header_start <= 1) break;

        char *colon = strchr(header_start, ':');
        if (colon && colon < line_end) {
            char key[64] = {0};
            size_t key_len = colon - header_start;
            if (key_len < sizeof(key))
                strncpy(key, header_start, key_len);

            char *value = colon + 1;
            while (*value == ' ' || *value == '\t') value++;

            if (strcmp(key, "Body-Size") == 0) {
                request->body_size = (int)strtol(value, NULL, 10);
                if (request->body_size < 0) request->body_size = 0;
            }
        }
        header_start = line_end + (line_end[0] == '\r' ? 2 : 1);
    }

}

char *get_status_text(HttpStatus status_code){
    switch (status_code) {
        case STATUS_OK:
            return "OK";
        case STATUS_CREATED:
            return "Created";
        case STATUS_NOT_FOUND:
            return "Not Found";
        case STATUS_BAD_REQUEST:
            return "Bad Request";
        case STATUS_SERVER_ERROR:
            return "Internal Server Error";
        default:
            return "Unknown Status";
    }
}

void process_request(HTTP_Request *req, HTTP_Response *resp) {
    resp->version = strdup("CHLP/1.0");
    resp->status_code = STATUS_BAD_REQUEST;  // default

    if (!req->version || strcmp(req->version, "CHLP/1.0") != 0) {
        resp->status_code = STATUS_BAD_REQUEST;
        return;
    }

    // Build safe file path (only once!)
    char filepath[1024] = {0};
    if (req->url[0] == '/' && req->method != HTTP_ECHO) {
        snprintf(filepath, sizeof(filepath), "./www%s", req->url);
        // Security check: no ".." allowed
        if (strstr(filepath, "..") != NULL) {
            resp->status_code = STATUS_BAD_REQUEST;
            return;
        }
    }

    switch (req->method) {
    case HTTP_GET:
        {
            FILE *file = fopen(filepath, "rb");
            if (!file) {
                resp->status_code = STATUS_NOT_FOUND;
                break;
            }
            
            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            resp->body = malloc(file_size + 1);
            if (!resp->body) {
                resp->status_code = STATUS_SERVER_ERROR;
                fclose(file);
                break;
            }
            
            size_t bytes_read = fread(resp->body, 1, file_size, file);
            if (bytes_read != file_size) {
                free(resp->body);
                resp->body = NULL;
                resp->status_code = STATUS_SERVER_ERROR;
                fclose(file);
                break;
            }
            resp->body[file_size] = '\0';
            resp->content_length = file_size;
            resp->status_code = STATUS_OK;
            fclose(file);
        }
        break;

    case HTTP_POST:
        printf("Received POST body: %s\n", req->body ? req->body : "(empty)");
        
        // Save POST body to the requested file (overwrites!)
        if (req->url[0] == '/' && req->body_size > 0) {
            FILE *f = fopen(filepath, "wb");
            if (f) {
                fwrite(req->body, 1, req->body_size, f);
                fclose(f);
            }
        }
        resp->status_code = STATUS_OK;
        break;

    case HTTP_ECHO:
        if (req->body) {
            resp->body = strdup(req->body);
            if (resp->body) {
                resp->content_length = req->body_size;
                resp->status_code = STATUS_OK;
            } else {
                resp->status_code = STATUS_SERVER_ERROR;
            }
        } else {
            resp->status_code = STATUS_OK;
            resp->content_length = 0;
        }
        break;

    default:
        resp->status_code = STATUS_BAD_REQUEST;
        break;
    }
}

void send_response(int client_fd, HTTP_Response *resp){
    char header[4096];
    int header_len = snprintf(header, sizeof(header), "%s %d %s\nBody-Size: %d\n\n", 
                              resp->version, resp->status_code, get_status_text(resp->status_code), 
                              resp->content_length);
    if (header_len < 0 || header_len >= sizeof(header)) {
        perror("snprintf error");
        return;
    }

    size_t full_len = header_len + (size_t)resp->content_length;

    char *response = malloc(full_len);
    if (!response) {
        perror("malloc error");
        return;
    }
    memcpy(response, header, header_len);
    if (resp->content_length > 0 && resp->body) {
        memcpy(response + header_len, resp->body, (size_t)resp->content_length);
    }

    size_t total_sent = 0;
    while (total_sent < full_len) {
        ssize_t sent = send(client_fd, response + total_sent, full_len - total_sent, 0);
        if (sent < 0) {
            perror("send error");
            break;
        }
        total_sent += (size_t)sent;
    }
    
    free(response);
}

void* handle_client(void *arg){ 
    int client_fd = *(int*)arg; free(arg);
    
    while (1) {  // Loop for multiple requests
        char buff[4096] = { 0 };
        int bytes_received = recv(client_fd, buff, sizeof(buff) - 1, 0);
        if (bytes_received <= 0) {  // Disconnect or error
            if (bytes_received == 0) printf("Client disconnected.\n");
            else perror("Recv error");
            break;
        }
        
        buff[bytes_received] = '\0';

        HTTP_Request req;
        parse_request(buff, &req);

        if (req.body_size > 0) {
            req.body = malloc(req.body_size + 1);
            if (!req.body) { perror("malloc body"); break; }  // Add check
            bytes_received = recv(client_fd, req.body, req.body_size, MSG_WAITALL);
            if (bytes_received <= 0) { free(req.body); break; }
            req.body[req.body_size] = '\0';
        }

        pthread_mutex_lock(&print_mutex);
        printf("--- PARSED REQUEST ---\n");
        printf("Raw Request:%s\r\n", buff);
        printf("Method (Enum): %d\r\n", req.method);
        printf("Path/URL: %s\r\n", req.url);
        printf("Version: %s\r\n", req.version);
        printf("This is the body.: %s\r\n", req.body);
        printf("----------------------\n");
        pthread_mutex_unlock(&print_mutex);
        fflush(stdout);

        HTTP_Response resp = {0};
        process_request(&req, &resp);
        send_response(client_fd, &resp);
        
        free(resp.body); free(resp.version);
        if (req.body) free(req.body);
        if (req.url) free(req.url);
        if (req.version) free(req.version);
    }

    close(client_fd);
    return NULL;  // Good practice
}