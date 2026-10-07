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
      char username[100];
char message[150];

printf("Enter username: ");
scanf("%99s", username);

snprintf(message, sizeof(message), "REGISTER %s\n", username);

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
  int c;
while ((c = getchar()) != '\n' && c != EOF)
    ;

char command[1024];

while (1)
{
     fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(client_fd, &readfds);

    int max_fd = client_fd;

    printf("> ");
    fflush(stdout);

    if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1)
    {
        perror("select");
        break;
    }

    if (FD_ISSET(client_fd, &readfds))
    {
        bytes_received = recv(client_fd, response,
                              sizeof(response) - 1, 0);

        if (bytes_received <= 0)
            break;

        response[bytes_received] = '\0';
        printf("\n%s", response);
    }

    if (FD_ISSET(STDIN_FILENO, &readfds))
    {
        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = '\0';

        if (strlen(command) == 0)
            continue;

        strcat(command, "\n");

        if (send(client_fd, command, strlen(command), 0) == -1)
        {
            perror("send");
            break;
        }

        if (strncmp(command, "QUIT", 4) == 0)
        {
            bytes_received = recv(client_fd, response,
                                  sizeof(response) - 1, 0);

            if (bytes_received > 0)
            {
                response[bytes_received] = '\0';
                printf("%s", response);
            }

            break;
        }
    }
} 

 return 0;
}
