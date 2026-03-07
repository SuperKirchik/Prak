#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <ctype.h>

#define MAX_CLIENTS 100
#define BUF_SIZE 1024
#define MAX_NICK_LEN 32
#define DEFAULT_PORT 8888

// Структура клиента
typedef struct {
    int fd;
    char nick[MAX_NICK_LEN];
    char buffer[BUF_SIZE];
    int buf_len;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int main_socket;

// Функции в программе
char *trim(char *str);
int find_free_slot();
int find_client_by_fd(int fd);
int is_nick_taken(const char *nick);
int count_active_clients();  
void broadcast_message(const char *msg, int exclude_fd);
void send_user_list(int fd);
void remove_client(int index, const char *quit_msg);
void process_client_message(int index);

// функции

// Подсчет активных клиентов
int count_active_clients() {
    int count = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active) count++;
    }
    return count;
}

// Очистка пробелов в начале и конце строки
char *trim(char *str) {
    char *end;
    
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

// Поиск свободного слота для клиента
int find_free_slot() {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(!clients[i].active) return i;
    }
    return -1;
}

// Поиск клиента по сокету
int find_client_by_fd(int fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && clients[i].fd == fd) return i;
    }
    return -1;
}

// Проверка, занят ли ник
int is_nick_taken(const char *nick) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && strcmp(clients[i].nick, nick) == 0) {
            return 1;
        }
    }
    return 0;
}

// Отправка сообщения всем клиентам
void broadcast_message(const char *msg, int exclude_fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && clients[i].fd != exclude_fd) {
            write(clients[i].fd, msg, strlen(msg));
        }
    }
}

// Отправка списка пользователей конкретному клиенту
void send_user_list(int fd) {
    char buffer[BUF_SIZE];
    int len = snprintf(buffer, BUF_SIZE, "*** Онлайн (%d): ", 
                       count_active_clients());  // <-- ТЕПЕРЬ ФУНКЦИЯ ОБЪЯВЛЕНА ВЫШЕ
    
    int first = 1;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active) {
            if(!first) {
                len += snprintf(buffer + len, BUF_SIZE - len, ", ");
            }
            len += snprintf(buffer + len, BUF_SIZE - len, "%s", clients[i].nick);
            first = 0;
        }
    }
    strncat(buffer, "\n", BUF_SIZE - len - 1);
    write(fd, buffer, strlen(buffer));
}

// Удаление клиента
void remove_client(int index, const char *quit_msg) {
    char buffer[BUF_SIZE];
    
    if(quit_msg && strlen(quit_msg) > 0) {
        snprintf(buffer, BUF_SIZE, "*** %s покинул чат: %s\n", 
                 clients[index].nick, quit_msg);
    } else {
        snprintf(buffer, BUF_SIZE, "*** %s покинул чат\n", 
                 clients[index].nick);
    }
    
    broadcast_message(buffer, clients[index].fd);
    
    close(clients[index].fd);
    clients[index].active = 0;
    clients[index].buf_len = 0;
    
    printf("Клиент %s отключился\n", clients[index].nick);
}

// Обработка входящего сообщения от клиента
void process_client_message(int index) {
    char buffer[BUF_SIZE];
    char *msg = clients[index].buffer;
    
    msg = trim(msg);
    
    if(strlen(msg) == 0) {
        clients[index].buf_len = 0;
        return;
    }
    
    if(msg[0] == '\\') {
        if(strncmp(msg, "\\users", 6) == 0) {
            send_user_list(clients[index].fd);
        }
        else if(strncmp(msg, "\\quit", 5) == 0) {
            char *quit_msg = msg + 5;
            while(isspace((unsigned char)*quit_msg)) quit_msg++;
            remove_client(index, quit_msg);
        }
        else {
            snprintf(buffer, BUF_SIZE, "*** Неизвестная команда. Доступно: \\users, \\quit [сообщение]\n");
            write(clients[index].fd, buffer, strlen(buffer));
        }
    }
    else {
        snprintf(buffer, BUF_SIZE, "%s: %s\n", clients[index].nick, msg);
        broadcast_message(buffer, -1);
    }
    
    clients[index].buf_len = 0;
    memset(clients[index].buffer, 0, BUF_SIZE);
}



int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    
    if(argc > 1) {
        port = atoi(argv[1]);
    }
    
    // Инициализация клиентов
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }
    
    // Создание сокета
    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(main_socket < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if(bind(main_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка bind");
        close(main_socket);
        exit(1);
    }
    
    if(listen(main_socket, 5) < 0) {
        perror("Ошибка listen");
        close(main_socket);
        exit(1);
    }
    
    printf("Чат-сервер запущен на порту %d\n", port);
    printf("Ожидание подключений...\n");
    
    memset(fds, 0, sizeof(fds));
    fds[0].fd = main_socket;
    fds[0].events = POLLIN;
    
    while(1) {
        nfds = 1;
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].active) {
                fds[nfds].fd = clients[i].fd;
                fds[nfds].events = POLLIN;
                fds[nfds].revents = 0;
                nfds++;
            }
        }
        
        int poll_result = poll(fds, nfds, -1);
        if(poll_result < 0) {
            if(errno == EINTR) continue;
            perror("Ошибка poll");
            break;
        }
        
        if(fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(main_socket, (struct sockaddr *)&client_addr, &addr_len);
            
            if(client_fd < 0) {
                perror("Ошибка accept");
            } else {
                int slot = find_free_slot();
                if(slot < 0) {
                    char *msg = "*** Сервер переполнен, попробуйте позже\n";
                    write(client_fd, msg, strlen(msg));
                    close(client_fd);
                } else {
                    char *ask = "*** Введите ваш ник: ";
                    write(client_fd, ask, strlen(ask));
                    
                    clients[slot].fd = client_fd;
                    clients[slot].buf_len = 0;
                    memset(clients[slot].buffer, 0, BUF_SIZE);
                    clients[slot].active = 1;
                    strcpy(clients[slot].nick, "???");
                    
                    printf("Новый клиент подключен, ожидание ника\n");
                }
            }
        }
        
        for(int i = 1; i < nfds; i++) {
            if(fds[i].revents & (POLLIN | POLLPRI)) {
                int client_fd = fds[i].fd;
                int client_idx = find_client_by_fd(client_fd);
                
                if(client_idx < 0) continue;
                
                char buf[BUF_SIZE];
                ssize_t n_read = read(client_fd, buf, BUF_SIZE - 1);
                
                if(n_read <= 0) {
                    if(clients[client_idx].nick[0] != '?') {
                        remove_client(client_idx, NULL);
                    } else {
                        close(client_fd);
                        clients[client_idx].active = 0;
                    }
                } else {
                    for(int j = 0; j < n_read; j++) {
                        if(buf[j] == '\n' || buf[j] == '\r') {
                            if(clients[client_idx].buf_len > 0) {
                                clients[client_idx].buffer[clients[client_idx].buf_len] = '\0';
                                
                                if(clients[client_idx].nick[0] == '?') {
                                    char *nick = trim(clients[client_idx].buffer);
                                    
                                    if(strlen(nick) == 0) {
                                        write(client_fd, "*** Ник не может быть пустым. Попробуйте снова: ", 48);
                                    } else if(strlen(nick) >= MAX_NICK_LEN) {
                                        write(client_fd, "*** Ник слишком длинный. Попробуйте снова: ", 44);
                                    } else if(is_nick_taken(nick)) {
                                        write(client_fd, "*** Ник уже занят. Выберите другой: ", 36);
                                    } else {
                                        strcpy(clients[client_idx].nick, nick);
                                        
                                        char welcome[BUF_SIZE];
                                        snprintf(welcome, BUF_SIZE, 
                                                "*** Добро пожаловать в чат, %s!\n"
                                                "*** Команды: \\users, \\quit [сообщение]\n", nick);
                                        write(client_fd, welcome, strlen(welcome));
                                        
                                        snprintf(welcome, BUF_SIZE, "*** %s вошел в чат\n", nick);
                                        broadcast_message(welcome, client_fd);
                                        
                                        printf("Клиент установил ник: %s\n", nick);
                                    }
                                } else {
                                    process_client_message(client_idx);
                                }
                            }
                            
                            clients[client_idx].buf_len = 0;
                            memset(clients[client_idx].buffer, 0, BUF_SIZE);
                        } else {
                            if(clients[client_idx].buf_len < BUF_SIZE - 1) {
                                clients[client_idx].buffer[clients[client_idx].buf_len++] = buf[j];
                            }
                        }
                    }
                }
            }
        }
    }
    
    close(main_socket);
    return 0;
}