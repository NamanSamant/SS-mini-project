#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "user.h"
#include "login.h"
#include "globals.h"


void logout_employee(int sock) {
	printf("\n");
	Request req;
	strcpy(req.action, "EMPLOYEE_LOGOUT");
	send(sock, &req, sizeof(Request), 0);
}

void add_new_customer(int sock) {
	printf("Adding a new Customer ...\n");
	Request req;
	char username[50];
	char password[50];
	printf("Enter Customer username: ");
	scanf("%s", username);
	printf("Enter Customer password: ");
	scanf("%s", password);
	User user;
	strcpy(user.username, username);
	strcpy(user.password, password);
	strcpy(req.action, "ADD_CUSTOMER");
	user.role = 1;
	req.user = user;
	send(sock, &req, sizeof(Request), 0);
	printf("Added new customer: %s\n",user.username);
}

void modify_customer_details(int sock) 
{
	int id;
	printf("Enter Customer Id: ");
	scanf("%d", &id);
	Request req;
	User user;
	strcpy(req.action, "GET_USER");
	req.user.id = id;
	send(sock, &req, sizeof(req), 0);
	ssize_t bytes_received = recv(sock, &user, sizeof(user), 0);
	if (bytes_received < 0) {
		perror("Receive failed");
		return;
	} else if (bytes_received == 0) {
		printf("Connection closed by peer\n");
		return;
	}
	strcpy(req.action, "MODIFY_CUSTOMER");
	char choice[5];
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
	req.user = user;
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Modification Failed \n");
	} else {
		printf("Modified Succesfully !!\n");
	}
}

void view_customer_transactions(int sock) {
    int id;
    printf("Enter Customer Id to view Transactions: ");
    scanf("%d", &id);
	FILE *file = fopen("database/transactions.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    Transaction trans;
    int i = 20;
    while (i && fscanf(file, "%d %d %f %s %s", &trans.from, &trans.to, &trans.amount, trans.from_username, trans.to_username) == 5) {
        if (trans.from == id || trans.to == id) {
            i--;
            if (trans.from== id && trans.to == id) {
                if (trans.amount < 0) {
                    printf("Withdrawn (%.2f)\n", trans.amount);
                } else {
                    printf("Deposited (%.2f)\n", trans.amount);
                }
            } else if (trans.from == id) { 
                printf("Transferred (%.2f) to %s\n", trans.amount, trans.to_username);
            } else if (trans.to == id) { 
                printf("Received (%.2f) from %s\n", trans.amount, trans.from_username);
            }
        }
    }
	fclose(file); 
}

void change_password_employee(int sock, User user) {
	char new_password[50];
	printf("Enter New password: ");
	scanf("%s", new_password);
	strcpy(user.password, new_password);
	Request req;
	req.user = user;
	strcpy(req.action, "CHANGE_PASSWORD");
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Password was not changed\n");
	} else {
		printf("Succesfull changed !!\n");
	}
}

void approve_reject_loans(int sock, User user){
	char status[20];
	int id;
	printf("Enter the loan id: ");
	scanf("%d", &id);
	printf("Enter the status(approved/rejected): ");
	scanf("%s", status);
	Request req;
	strcpy(req.status, status);
	req.user.id = id;
	strcpy(req.action, "STATUS_UPDATE");
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("status was not updated\n");
	} else {
		printf("Succesfully updated status !!\n");
	}	
}

void view_assigned_loans(int sock, User user){
	FILE *file = fopen("database/loans.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        return;
    }
    Loan loan;
    int found = 0;
    printf("\nLoans assigned to you-----\n");
    while (fscanf(file, "%d %s %s %s %f", &loan.id, loan.customer, loan.handler, loan.status, &loan.amount) != EOF) {
        if (strcmp(loan.handler, user.username) == 0) {
            printf("Loan id: %d\nCustomer: %s\nLoan status: %s\nLoan Amount: %.2f\n", loan.id, loan.customer, loan.status, loan.amount);
            found = 1;
        }
    }
    fclose(file);
    if (!found) {
        printf("No loans assigned to you\n");
    }
}

void employee_menu(int sock, User user) {
	while (1) {
		printf("\nEmployee Menu:\n");
		printf("1. Add New Customer\n");
		printf("2. Modify Customer Details\n");
		printf("3. Approve/Reject Loans\n");
		printf("4. View Assigned Loan Applications\n");
		printf("5. View Customer Transactions\n");
		printf("6. Change Password\n");
		printf("7. Logout\n");
		printf("8. Exit\n");

		int choice;
		printf("\nEnter your choice: ");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			add_new_customer(sock);
			break;
		case 2:
			modify_customer_details(sock);
			break;
		case 3:
			approve_reject_loans(sock, user);
			break;
		case 4:
			view_assigned_loans(sock, user);
			break;
		case 5:
			view_customer_transactions(sock);
			break;
		case 6:
			change_password_employee(sock, user);
			break;
		case 7:
			logout_employee(sock);
			return;
		case 8:
			printf("Exiting...\n");
			exit(0);
		default:
			printf("Invalid choice. Please try again.\n");
		}
	}

}