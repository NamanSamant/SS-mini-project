#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include "banking.h"
#include "login.h"
#include "admin.h"
#include "user.h"
#include "globals.h"

User current_user;

void addTransaction(Transaction trans) {
    FILE *file = fopen("database/transactions.txt", "a");  
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    fprintf(file, "%d %d %.2f %s %s\n", trans.from, trans.to, trans.amount, trans.from_username, trans.to_username);
    fflush(file);
    fclose(file);
}

User getUser(int id) {
    FILE *file = fopen("database/users.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        User empty_user = {0};  
        return empty_user;
    }

    User user;
    int found = 0;
    char line[100];  
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password,&user.role,&user.account_balance);
        if (user.id == id) {
            found = 1;
            break;
        }
    }
    fclose(file);
    if (!found) {
        printf("User with ID %d not found.\n", id);
        User empty_user = {0};
        return empty_user;
    }
    return user;
}

void admin_functions(Request req, int client_sock) {
    if (strcmp(req.action, "ADD_EMPLOYEE") == 0) {
        add_user(req.user.username, req.user.password, req.user.role);
        printf("User %s added.\n", req.user.username);
    }
    if (strcmp(req.action, "MODIFY_EMPLOYEE") == 0) {
        modify_user(req.user);
        printf("User %s modified.\n", req.user.username);
    }
    if (strcmp(req.action, "GET_USER") == 0) {
        User user = getUser(req.user.id);
        send(client_sock, &user, sizeof(User), 0);
    }
}

void customer_functions(Request req, int client_sock) {
    printf("%s\n", req.action);
    if (strcmp(req.action, "GET_BALANCE") == 0) {
        User user = get_balance(req.user);
        send(client_sock, &user, sizeof(user), 0);
    }

    if (strcmp(req.action, "LOAN") == 0) {
        loanapplication(req.user, req.amount);
    }

    if (strcmp(req.action, "FEEDBACK") == 0) {
        addFeedback(req.user, req.feedback);
    }

    if (strcmp(req.action, "DEPOSIT") == 0) {
        deposit(req.user, req.amount);
        User user = req.user;
        Transaction trans;
        trans.to = user.id;
        trans.from = user.id;
        trans.amount = req.amount;
        strcpy(trans.from_username, user.username);
        strcpy(trans.to_username,user.username);
        addTransaction(trans);
    }

    if (strcmp(req.action, "TRANSFER_FUNDS") == 0) {
        User destination = getUser(req.transfer_id);
        User source = getUser(req.user.id);
        char error[50];
        if (source.account_balance < req.amount) {
            printf("Error doing transaction due to insuffienct funds.\n");
            strcpy(error, "insuffienct funds");
            send(client_sock, error, sizeof(error), 0);
            return;
        }
        deposit(destination, req.amount);
        deposit(req.user, -1.00 * req.amount);
        Transaction trans;
        trans.to = destination.id;
        trans.from = source.id;
        trans.amount = -1 * req.amount;
        strcpy(trans.from_username, source.username);
        strcpy(trans.to_username, destination.username);
        addTransaction(trans);
        trans.to = source.id;
        trans.from = destination.id;
        trans.amount = req.amount;
        strcpy(trans.from_username, destination.username);
        strcpy(trans.to_username, source.username);
        addTransaction(trans);
        strcpy(error, "no error");
        send(client_sock, error, sizeof(error), 0);
    }

    if (strcmp(req.action, "CHANGE_PASSWORD") == 0) {
        char new_password[50];
        strcpy(new_password, req.user.password);
        User user = getUser(req.user.id);
        strcpy(user.password, new_password);
        update_password(user);
    }
}

void employee_functions(Request req, int client_sock) {
    if (strcmp(req.action, "ADD_CUSTOMER") == 0) {
        add_user(req.user.username, req.user.password, req.user.role);
        printf("User %s added.\n", req.user.username);
        return;
    }
    if (strcmp(req.action, "GET_USER") == 0) {
        User user = getUser(req.user.id);
        send(client_sock, &user, sizeof(user), 0);
    }
    if(strcmp(req.action, "MODIFY_CUSTOMER") == 0) {
        modify_user(req.user);
        printf("Customer Modified \n");
    }
    if(strcmp(req.action, "STATUS_UPDATE") == 0) {
        approveRejectLoan(req.user.id, req.status);
    }
    if (strcmp(req.action, "CHANGE_PASSWORD") == 0) {
        char new_password[50];
        strcpy(new_password, req.user.password);
        User user = getUser(req.user.id);
        strcpy(user.password, new_password);
        update_password(user);
    }
}

void manager_function(Request req, int client_sock) {
    if (strcmp(req.action, "DEACTIVATE_ACC") == 0) {
        User user = getUser(req.user.id);
        user.account_balance = -1.0;
        modify_user(user);
    }
    if (strcmp(req.action, "ACTIVATE_ACC") == 0) {
        User user = getUser(req.user.id);
        if(user.account_balance == -1) user.account_balance = 0.0;
        modify_user(user);
    }
    if (strcmp(req.action, "LOAN_ASSIGN") == 0) {
        loanAssign(req.user.id, req.user.username);
    }
    if (strcmp(req.action, "REVIEW_FEEDBACK") == 0) {
        int id = req.user.id;
        feedbackReview(id);
    }
    if (strcmp(req.action, "CHANGE_PASSWORD") == 0) {
        char new_password[50];
        strcpy(new_password, req.user.password);
        User user = getUser(req.user.id);
        strcpy(user.password, new_password);
        update_password(user);
    }
}

#define PORT 8080

void* handle_client(void* socket_desc) {
    int client_sock = *(int*)socket_desc;
    free(socket_desc);
    while (1) {
        char username[50];
        char password[50];
        recv(client_sock, username, sizeof(username), 0);
        recv(client_sock, password, sizeof(password), 0);
        if (login(username, password)) {
            send(client_sock, &current_user, sizeof(current_user), 0);
            Request req;
            while (1) {
                ssize_t bytes_received = recv(client_sock, &req, sizeof(Request), 0);
                if (bytes_received < 0) {
                    perror("Receive failed");
                    return NULL; 
                } else if (bytes_received == 0) {
                    printf("Connection closed by peer\n");
                    return NULL; 
                }
                if (current_user.role == 4) {
                    if (strcmp(req.action, "ADMIN_LOGOUT") == 0) {
                        current_user.id = -1;
                        break;
                    }
                    admin_functions(req, client_sock);
                } else if (current_user.role == 1) {
                    if (strcmp(req.action, "CUSTOMER_LOGOUT") == 0) {
                        current_user.id = -1;
                        break;
                    }
                    customer_functions(req, client_sock);
                } else if(current_user.role == 2) {
                    if (strcmp(req.action, "EMPLOYEE_LOGOUT") == 0) {
                        current_user.id = -1;
                        break;
                    }
                    employee_functions(req, client_sock);
                } else {
                    if (strcmp(req.action, "MANAGER_LOGOUT") == 0) {
                        current_user.id = -1;
                        break;
                    }
                    manager_function(req, client_sock);
                }
            }
        } else {
            send(client_sock, &current_user, sizeof(current_user), 0);
        }
    }
    close(client_sock);
    pthread_exit(NULL); 
}

int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    listen(server_sock, 5);
    printf("Server is listening on port %d\n", PORT);
    while ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len))) {
        printf("Connection accepted\n");
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Could not create thread");
            free(new_sock);
            close(client_sock);
        }
        pthread_detach(client_thread);
    }

    if (client_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    close(server_sock);
    return 0;
}

