#include "http.h"

struct sockaddr_in server;
const char* ip = "127.0.0.1";

int main(){
    int sockfd = connect_to_server(PORT, &server, ip);
    if (sockfd == -1)
    {
        perror("Connect to server error\n");
        exit(1);
    }

    char input[32];
    while (1)
    {
        printf("\n=== CHLP Client ===\n");
        printf("Available methods: GET, POST, ECHO, QUIT\n");
        printf("Enter method: ");

        if (!fgets(input, sizeof(input), stdin)) break;
        for (char *p = input; *p; p++) *p = toupper(*p);
        input[strcspn(input, "\n")] = '\0';

        if (strcasecmp(input, "QUIT") == 0 || strcasecmp(input, "Q") == 0) {
            break;
        }

        char resource[256] = "/";
        char body[4096] = "";
        size_t body_size = 0;

        if (strcasecmp(input, "GET") == 0) {
            printf("Enter resource (e.g. /hello.txt): ");
            fgets(resource, sizeof(resource), stdin);
            resource[strcspn(resource, "\n")] = '\0';
            if (resource[0] != '/')
            {
                strcpy(resource + 1, resource);
                resource[0] = '/';
            }
            body_size = 0;
        }else if (strcasecmp(input, "POST") == 0) {
            printf("Enter resource (e.g. /upload): ");
            fgets(resource, sizeof(resource), stdin);
            resource[strcspn(resource, "\n")] = '\0';
            if (resource[0] != '/')
            {
                strcpy(resource + 1, resource);
                resource[0] = '/';
            }
            printf("Enter body (one line): ");
            fgets(body, sizeof(body), stdin);
            body[strcspn(body, "\n")] = '\0';
            body_size = strlen(body);
        }else if (strcasecmp(input, "ECHO") == 0) {
            printf("Enter text to echo: ");
            fgets(body, sizeof(body), stdin);
            body[strcspn(body, "\n")] = '\0';
            body_size = strlen(body);
            strcpy(resource, "/");
        }else {
            printf("Unknown method!\n");
            continue;
        }

        char headers[1024];
        snprintf(headers, sizeof(headers),
            "%s %s CHLP/1.0\n"
            "Body-Size: %zu\n"
            "\n",
            input, resource, body_size);

        printf("\n--- Sending Request ---\n%s", headers);
        if (body_size > 0) printf("Body: %s\n", body);

        send(sockfd, headers, strlen(headers), 0);
        if (body_size > 0) {
            send(sockfd, body, body_size, 0);
        }

        char resp_buf[8192] = { 0 };
        int len = recv(sockfd, resp_buf, sizeof(resp_buf) - 1, 0);
        if (len > 0) {
            resp_buf[len] = '\0';
            printf("\n--- Response ---\n%s\n", resp_buf);
        }
        else if (len == 0) {
            printf("Server closed connection.\n");
        }
        else {
            perror("recv");
        }

        printf("\nPress Enter to continue...");
        getchar();
    }

    close(sockfd);
    printf("Goodbye!\n");
    return 0;
}

    /*char* headers = "ECHO / CHLP/1.0\n"
                "Body-Size: 5\n"
                "\n";

    printf("Sending Headers...\n");
    if (send(sockfd, headers, strlen(headers), 0) <= 0) {
        perror("Send headers error");
        close(sockfd);
        exit(1);
    }

    sleep(1);

    char *body = "Hello";
    printf("Sending Body...\n");
    if (send(sockfd, body, strlen(body), 0) <= 0) {
        perror("Send body error");
    }
    
    char resp_buf[4096] = {0};
    int recv_len = recv(sockfd, resp_buf, sizeof(resp_buf)-1, 0);
    if (recv_len > 0) printf("Response: %s\n", resp_buf);
    
    printf("Response received. Closing.\n");
    close(sockfd);
    return 0;
}*/