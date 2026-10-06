#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <pthread.h>

#define PORT 11870
#define BACKLOG 10
#define MAX_CLIENTS 5

char usernames[MAX_CLIENTS][100];
int user_count = 0;

void *handle_client(void *arg);
void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[256];
    ssize_t bytes_received;

    bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received == -1)
    {
        perror("recv");
        close(client_fd);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    printf("Received: %s", buffer);

    char username[100];
    char response[256];

    if (sscanf(buffer, "REGISTER %99s", username) != 1)
    {
        snprintf(response, sizeof(response),
                 "ERR 001 INVALID_REGISTER NID:6958\n");
    }
    else
    {
        snprintf(response, sizeof(response),
                 "OK REGISTERED %s NID:6958\n", username);
    }

    if (send(client_fd, response, strlen(response), 0) == -1)
    {
        perror("send");
        close(client_fd);
        return NULL;
    }

    printf("Response sent: %s", response);

    close(client_fd);

    return NULL;
}



int main(void)
{
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

  
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) == -1)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d.\n", PORT);
    printf("Server socket created successfully.\n");

    while(1)
    {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;
    
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);

    if (client_fd == -1)
    {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected successfully.\n");
   int *client_socket = malloc(sizeof(int));
*client_socket = client_fd;

pthread_t thread;

if (pthread_create(&thread, NULL, handle_client, client_socket) != 0)
{
    perror("pthread_create");
    close(client_fd);
    free(client_socket);
    continue;
}

pthread_detach(thread);
       
}

return 0;
}


    

