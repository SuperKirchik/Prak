#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#define BUF_SIZE 1024
#define DEFAULT_PORT 8888
#define DEFAULT_HOST "127.0.0.1"

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char host[256] = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    struct pollfd fds[2];
    int running = 1;
    
    if(argc > 1) strncpy(host, argv[1], sizeof(host) - 1);
    if(argc > 2) port = atoi(argv[2]);
    
    printf("Подключение к чат-серверу %s:%d...\n", host, port);
    
    // Создание сокета
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }
    
    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Ошибка преобразования адреса");
        close(sock);
        exit(1);
    }
    
    // Подключение
    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка подключения");
        close(sock);
        exit(1);
    }
    
    printf("Подключено к серверу!\n");
    
    // Настройка poll
    fds[0].fd = 0;  // stdin
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;
    
    while(running) {
        int poll_result = poll(fds, 2, -1);
        
        if(poll_result < 0) {
            perror("Ошибка poll");
            break;
        }
        
        // Данные от сервера
        if(fds[1].revents & POLLIN) {
            ssize_t n_read = read(sock, recv_buf, BUF_SIZE - 1);
            
            if(n_read <= 0) {
                printf("\nСервер отключился\n");
                break;
            }
            
            recv_buf[n_read] = '\0';
            printf("%s", recv_buf);
            fflush(stdout);
        }
        
        // Ввод пользователя
        if(fds[0].revents & POLLIN) {
            if(fgets(send_buf, BUF_SIZE, stdin) == NULL) {
                break;
            }
            
            // Отправляем серверу
            write(sock, send_buf, strlen(send_buf));
            
            // Проверяем команду выхода
            if(strncmp(send_buf, "\\quit", 5) == 0) {
                printf("Выход из чата...\n");
                running = 0;
                break;
            }
        }
    }
    
    close(sock);
    return 0;
}