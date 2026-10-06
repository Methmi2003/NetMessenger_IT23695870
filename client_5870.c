#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 11870

int main(void)
{
    int client_fd;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("Client socket created successfully.\n");
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server successfully.\n");
      char message[] = "REGISTER Nimal\n";

    if (send(client_fd, message, strlen(message), 0) == -1)
    {
        perror("send");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("REGISTER command sent.\n"); 
    char response[256];
    ssize_t bytes_received;

    bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);

    if (bytes_received == -1)
    {
        perror("recv");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    response[bytes_received] = '\0';

    printf("Server response: %s", response);
    char list_message[] = "LIST\n";

if (send(client_fd, list_message, strlen(list_message), 0) == -1)
{
    perror("send");
    close(client_fd);
    exit(EXIT_FAILURE);
}

printf("LIST command sent.\n");

bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);

if (bytes_received == -1)
{
    perror("recv");
    close(client_fd);
    exit(EXIT_FAILURE);
}

response[bytes_received] = '\0';

printf("LIST response: %s", response);
 
char bcast_message[] = "BCAST Hello everyone!\n";

if (send(client_fd, bcast_message, strlen(bcast_message), 0) == -1)
{
    perror("send");
    close(client_fd);
    exit(EXIT_FAILURE);
}

printf("BCAST command sent.\n");

bytes_received = recv(client_fd, response, sizeof(response) - 1, 0);

if (bytes_received == -1)
{
    perror("recv");
    close(client_fd);
    exit(EXIT_FAILURE);
}

response[bytes_received] = '\0';

printf("BCAST response: %s", response);

   close(client_fd);

 return 0;
}
