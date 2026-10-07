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
#include <sys/stat.h>

#define PORT 11870
#define BACKLOG 10
#define MAX_CLIENTS 5

char usernames[MAX_CLIENTS][100];
int user_count = 0;
int client_sockets[MAX_CLIENTS];

int client_number = 0;

#define MAX_ROOMS 10

char room_names[MAX_ROOMS][100];
int room_members[MAX_ROOMS][MAX_CLIENTS];
int room_count = 0;

pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

int recv_line(int fd, char *buffer, int size)
{
    int i = 0;
    char ch;

    while (i < size - 1)
    {
        int n = recv(fd, &ch, 1, 0);

        if (n <= 0)
        {
            return n;
        }

        buffer[i++] = ch;

        if (ch == '\n')
        {
            break;
        }
    }

    buffer[i] = '\0';
    return i;
}

int recv_all(int fd, char *buffer, int size)
{
    int total = 0;

    while (total < size)
    {
        int n = recv(fd, buffer + total, size - total, 0);

        if (n <= 0)
        {
            return n;
        }

        total += n;
    }

    return total;
}

void *handle_client(void *arg);
void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);


    char buffer[256];
    ssize_t bytes_received;
    char registered_username[100] = "";

   while(1)
{

   bytes_received = recv_line(client_fd, buffer, sizeof(buffer));

    if (bytes_received == -1)
    {
        perror("recv");
        close(client_fd);
        return NULL;
    }

    if (bytes_received == 0)
{
pthread_mutex_lock(&user_mutex);

for (int i = 0; i < user_count; i++)
{
    if (strcmp(usernames[i], registered_username) == 0)
    {
        for (int j = i; j < user_count - 1; j++)
        {
            strcpy(usernames[j], usernames[j + 1]);
            client_sockets[j] = client_sockets[j + 1];
        }

        user_count--;
        break;
    }
}

pthread_mutex_unlock(&user_mutex);




    close(client_fd);
    return NULL;

}
    buffer[bytes_received] = '\0';

    printf("Received: %s", buffer);

    
    char username[100];
    char response[256];

    if (strncmp(buffer, "REGISTER ", 9) == 0 && sscanf(buffer, "REGISTER %99s", username) != 1)
    {
        snprintf(response, sizeof(response),
                 "ERR 001 INVALID_REGISTER NID:6958\n");
    }
    else
    {


int username_taken = 0;

pthread_mutex_lock(&user_mutex);

for (int i = 0; i < user_count; i++)
{
    if (strcmp(usernames[i], username) == 0)
    {
        username_taken = 1;
        break;
    }
}

if (username_taken)
{
    strcpy(response, "ERR 001 USERNAME_TAKEN NID:6958\n");
}
else
{
    snprintf(response, sizeof(response),
             "OK REGISTERED %s NID:6958\n", username);

    if (user_count < MAX_CLIENTS)
    {
        strcpy(usernames[user_count], username);
        client_sockets[user_count] = client_fd;
        user_count++;
    }

    strcpy(registered_username, username);
}

pthread_mutex_unlock(&user_mutex);
    }
if (strncmp(buffer, "LIST", 4) == 0)
{
    pthread_mutex_lock(&user_mutex);

    strcpy(response, "OK USERS ");

    for (int i = 0; i < user_count; i++)
    {
        strcat(response, usernames[i]);

        if (i < user_count - 1)
        {
            strcat(response, ",");
        }
 }

    strcat(response, " NID:6958\n");

    pthread_mutex_unlock(&user_mutex);
}

if (strncmp(buffer, "PMSG ", 5) == 0)
{
    char target_username[100];
    char message[200];
    char private_message[256];
    int target_fd = -1;

    if (sscanf(buffer, "PMSG %99s %199[^\n]",
               target_username, message) == 2)
    {
        pthread_mutex_lock(&user_mutex);

        for (int i = 0; i < user_count; i++)
        {
            if (strcmp(usernames[i], target_username) == 0)
            {
                target_fd = client_sockets[i];
                break;
            }
        }

        pthread_mutex_unlock(&user_mutex);

        if (target_fd == -1)
        {
            strcpy(response,
                   "ERR 002 USER_NOT_FOUND NID:6958\n");
        }
        else
        {
            snprintf(private_message,
                     sizeof(private_message),
                     "MSG PRIV %s %s\n",
                     registered_username, message);

            send(target_fd, private_message,
                 strlen(private_message), 0);

            strcpy(response, "OK SENT NID:6958\n");
        }
    }
}

if (strncmp(buffer, "JOIN ", 5) == 0)
{
    char room[100];
    int room_index = -1;

    if (sscanf(buffer, "JOIN %99s", room) == 1)
    {
        pthread_mutex_lock(&user_mutex);

        for (int i = 0; i < room_count; i++)
        {
            if (strcmp(room_names[i], room) == 0)
            {
                room_index = i;
                break;
            }
        }

        if (room_index == -1 && room_count < MAX_ROOMS)
        {
            room_index = room_count;
            strcpy(room_names[room_count], room);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                room_members[room_count][i] = -1;
            }

            room_count++;
        }

        if (room_index != -1)
        {
            int already_member = 0;

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (room_members[room_index][i] == client_fd)
                {
                    already_member = 1;
                    break;
                }
            }

            if (!already_member)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (room_members[room_index][i] == -1)
                    {
                        room_members[room_index][i] = client_fd;
                        break;
                    }
                }
            }

            snprintf(response, sizeof(response),
                     "OK JOINED %s NID:6958\n", room);
        }

        pthread_mutex_unlock(&user_mutex);
    }
}

if (strncmp(buffer, "ROOMS", 5) == 0)
{
    pthread_mutex_lock(&user_mutex);

    strcpy(response, "OK ROOMS ");

    for (int i = 0; i < room_count; i++)
    {
        strcat(response, room_names[i]);

        if (i < room_count - 1)
        {
            strcat(response, ",");
        }
    }

    strcat(response, " NID:6958\n");

    pthread_mutex_unlock(&user_mutex);
}

if (strncmp(buffer, "LEAVE ", 6) == 0)
{
    char room[100];
    int room_index = -1;

    if (sscanf(buffer, "LEAVE %99s", room) == 1)
    {
        pthread_mutex_lock(&user_mutex);

        for (int i = 0; i < room_count; i++)
        {
            if (strcmp(room_names[i], room) == 0)
            {
                room_index = i;
                break;
            }
        }

        if (room_index == -1)
        {
            strcpy(response,
                   "ERR 003 ROOM_NOT_FOUND NID:6958\n");
        }
        else
        {
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (room_members[room_index][i] == client_fd)
                {
                    room_members[room_index][i] = -1;
                    break;
                }
            }

            snprintf(response, sizeof(response),
                     "OK LEFT %s NID:6958\n", room);
        }

        pthread_mutex_unlock(&user_mutex);
    }
}

if (strncmp(buffer, "RMSG ", 5) == 0)
{
    char room[100];
    char message[200];
    char room_message[256];
    int room_index = -1;

    if (sscanf(buffer, "RMSG %99s %199[^\n]",
               room, message) == 2)
    {
        pthread_mutex_lock(&user_mutex);

        for (int i = 0; i < room_count; i++)
        {
            if (strcmp(room_names[i], room) == 0)
            {
                room_index = i;
                break;
            }
        }

        if (room_index == -1)
        {
            strcpy(response,
                   "ERR 003 ROOM_NOT_FOUND NID:6958\n");
        }
        else
        {
            snprintf(room_message,
                     sizeof(room_message),
                     "MSG ROOM %s %s %s\n",
                     room, registered_username, message);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (room_members[room_index][i] != -1)
                {
                    send(room_members[room_index][i],
                         room_message,
                         strlen(room_message), 0);
                }
            }

            strcpy(response, "OK SENT NID:6958\n");
        }

        pthread_mutex_unlock(&user_mutex);
    }
}

if (strncmp(buffer, "BCAST ", 6) == 0)



{
    char message[200];
    char broadcast_message[256];

    strcpy(message, buffer + 6);

    message[strcspn(message, "\n")] = '\0';

    snprintf(broadcast_message, sizeof(broadcast_message),
             "MSG BCAST %s %s\n", registered_username, message);

    pthread_mutex_lock(&user_mutex);

    for (int i = 0; i < user_count; i++)
    {
        if (client_sockets[i] != client_fd)
        {
            send(client_sockets[i], broadcast_message,
                 strlen(broadcast_message), 0);
        }
    }

    pthread_mutex_unlock(&user_mutex);

    strcpy(response, "OK SENT NID:6958\n");
}

if (strncmp(buffer, "SENDFILE ", 9) == 0)
{
    char target[100];
    char filename[100];
    int filesize;

    if (sscanf(buffer, "SENDFILE %99s %99s %d\n",
               target, filename, &filesize) == 3)
    {
        if (filesize > 1048576)
        {
            strcpy(response,
                   "ERR 004 FILE_TOO_LARGE NID:6958\n");
        }
        else
        {
            int target_fd = -1;

            pthread_mutex_lock(&user_mutex);

            for (int i = 0; i < user_count; i++)
            {
                if (strcmp(usernames[i], target) == 0)
                {
                    target_fd = client_sockets[i];
                    break;
                }
            }

            pthread_mutex_unlock(&user_mutex);

            if (target_fd == -1)
            {
                strcpy(response,
                       "ERR 002 USER_NOT_FOUND NID:6958\n");
            }
            else
            {
                char *file_data = malloc(filesize);

                if (file_data == NULL)
                {
                    strcpy(response,
                           "ERR 004 FILE_TOO_LARGE NID:6958\n");
                }
                else
                {
                    int received = recv_all(client_fd,
                                            file_data,
                                            filesize);

                    if (received == filesize)
                    {

                        char sender_dir[256];
char file_path[512];

snprintf(sender_dir, sizeof(sender_dir),
         "storage/IT23695870/%s",
         registered_username);

mkdir("storage/IT23695870", 0777);
mkdir(sender_dir, 0777);

snprintf(file_path, sizeof(file_path),
         "%s/%s",
         sender_dir, filename);

FILE *fp = fopen(file_path, "wb");

if (fp != NULL)
{
    fwrite(file_data, 1, filesize, fp);
    fclose(fp);
}
                        send(target_fd, file_data, filesize, 0);

                        strcpy(response,
                               "OK FILE_RECEIVED ");
                        strcat(response, filename);
                        strcat(response, " NID:6958\n");
                    }

                    free(file_data);
                }
            }
        }
    }
    else
    {
        strcpy(response,
               "ERR 001 INVALID_SENDFILE NID:6958\n");
    }
}
    if (send(client_fd, response, strlen(response), 0) == -1)
    {
        perror("send");
        close(client_fd);
        return NULL;
    }

    printf("Response sent: %s", response);

   }

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

    client_number++;
printf("Client %d connected successfully.\n", client_number);
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


    

