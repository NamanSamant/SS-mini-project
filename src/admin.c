#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "admin.h"
#include "login.h"
#include "globals.h"
#include "user.h"

// =========================================================================================================================================================
// ADMIN RELATED FUNCTIONALITIES

void add_new_employee(int sock) {
    printf("Adding a new bank employee...\n");
    Request req;
    char username[50];
    char password[50];
    int role;
    printf("Enter Employee username: ");
    scanf("%s", username);
    printf("Enter Employee password: ");
    scanf("%s", password);
    printf("Enter Role: ");
    scanf("%d", &role);
    User user;
    strcpy(user.username, username);
    strcpy(user.password, password);
    strcpy(req.action, "ADD_EMPLOYEE");
    user.role = role;
    req.user = user;
    send(sock, &req, sizeof(Request), 0);
    printf("Added new employee with employee name : %s\n", user.username);
}

void modify_customer_employee_details(int sock) 
{
    int id;
    printf("Modifying customer/employee details...\n");
    printf("Enter ID of user to be modified: ");
    scanf("%d",&id);
    if(id==1){
        printf("You cannot modify the admin details\n");
        return;
    }
    Request req;
    User user;
    strcpy(req.action, "GET_USER");
    req.user.id = id;
    send(sock, &req, sizeof(req), 0);
    ssize_t bytes_received = recv(sock, &user, sizeof(User), 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        return; 
    } else if (bytes_received == 0) {
        printf("Connection closed by peer\n");
        return; 
    }
    char choice[5];
    strcpy(req.action, "MODIFY_EMPLOYEE");
    user.id = id;
    printf("Modify username? (yes/no): ");
    scanf("%s", choice);
    if (strcmp(choice, "yes") == 0) {
        printf("Enter new username: ");
        scanf("%s", user.username);
    }
    printf("Modify password? (yes/no): ");
    scanf("%s", choice);
    if (strcmp(choice, "yes") == 0) {
        printf("Enter new password: ");
        scanf("%s", user.password);
    }
    printf("Modify role? (yes/no): ");
    scanf("%s", choice);
    if (strcmp(choice, "yes") == 0) {
        printf("Enter new role (1: customer, 2: employee, 3: manager, 4: admin): ");
        scanf("%d", &user.role);
    }
    printf("Modify account balance? (yes/no): ");
    scanf("%s", choice);
    if (strcmp(choice, "yes") == 0) {
        printf("Enter new account balance: ");
        scanf("%f", &user.account_balance);
    }
    req.user = user;
    send(sock, &req, sizeof(Request), 0);
    printf("Modified the user details succesfully for user: %s\n",user.username);
}

void logout_admin(int sock) {
    printf("\n");
    Request req;
    strcpy(req.action, "ADMIN_LOGOUT");
    send(sock, &req, sizeof(Request), 0);
}

// =========================================================================================================================================================
// ADMIN MENU

void admin_menu(int sock, User user) {
    while (1) {
        printf("\n Admin Menu:\n");
        printf("1. Add New Bank Employee\n");
        printf("2. Modify Customer/Employee Details\n");
        printf("3. Logout\n");
        printf("4. Print all User\n");
        printf("5. Exit\n");

        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            add_new_employee(sock);
            break;
        case 2:
            modify_customer_employee_details(sock);
            break;
        case 3:
            logout_admin(sock);
            return;
        case 4:
            printAllUser();
            break;
        case 5:
            printf("Exiting...\n");
            exit(0); 
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
}





