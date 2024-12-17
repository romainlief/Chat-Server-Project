#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>

#define MAX_PSEUDO_LEN 30
#define MAX_MSG_LEN 1024



int sockfd;
char pseudo[MAX_PSEUDO_LEN];
volatile int running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nDéconnexion...\n");
        running = 0;
        close(sockfd);
        exit(0);
    }
}

void *receive_messages(void *arg) {
    char buffer[MAX_MSG_LEN];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            printf("%s\n", buffer);
        } else if (bytes_received == 0) {
            printf("Déconnecté du serveur.\n");
            running = 0;
            break;
        } else if (errno != EINTR) {
            perror("recv");
            running = 0;
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage : ./chat PSEUDO\n");
        return 1;
    }

    if (strlen(argv[1]) >= MAX_PSEUDO_LEN) {
        fprintf(stderr, "Erreur : pseudonyme trop long (max %d caractères).\n", MAX_PSEUDO_LEN);
        return 2;
    }
    strcpy(pseudo, argv[1]);

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    const char *ip = getenv("IP_SERVEUR") ? getenv("IP_SERVEUR") : "127.0.0.1";
    int port = getenv("PORT_SERVEUR") ? atoi(getenv("PORT_SERVEUR")) : 1234;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    send(sockfd, pseudo, strlen(pseudo), 0);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("pthread_create");
        close(sockfd);
        return 1;
    }

    char message[MAX_MSG_LEN];
    while (running) {
        printf("> ");
        if (fgets(message, sizeof(message), stdin) == NULL) break;

        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') message[len - 1] = '\0';

        char formatted_message[MAX_MSG_LEN + MAX_PSEUDO_LEN + 3];
        snprintf(formatted_message, sizeof(formatted_message), "[%s] %s", pseudo, message);

        if (send(sockfd, formatted_message, strlen(formatted_message), 0) < 0) {
            perror("send");
            break;
        }
    }

    running = 0;
    pthread_join(recv_thread, NULL);
    close(sockfd);
    return 0;
}
