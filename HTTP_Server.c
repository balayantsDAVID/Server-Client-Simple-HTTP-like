#include "http.h"   

struct sockaddr_in server;

int main(){
    int socket_fd = server_create(PORT, &server);
    if (socket_fd == -1)
    {
        perror("Server create error\n");
        exit(1);
    }
    
    while (1) {
        int client_fd = accept(socket_fd, NULL, NULL);
        if (client_fd < 0) 
        {   
            perror("accept"); 
            continue;
        }

        int *fd_ptr = malloc(sizeof(int));
        *fd_ptr = client_fd;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, fd_ptr);
        pthread_detach(thread);
    }
    
    close(socket_fd);
    return 0;
}


