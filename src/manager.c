#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "user.h"
#include "login.h"
#include "globals.h"

void deactivate_customer_account(int sock) {
	Request req;
	int id;
	printf("Enter the Customer ID to deactivate its account: ");
	scanf("%d", &id);
	strcpy(req.action, "DEACTIVATE_ACC");
	req.user.id = id;
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Deactivation Failed \n");
	} else {
		printf("Deactivated Succesfully !!\n");
	}
}

void activate_customer_account(int sock) {
	Request req;
	int id;
	printf("Enter the Customer ID to activate its account: ");
	scanf("%d", &id);
	strcpy(req.action, "ACTIVATE_ACC");
	req.user.id = id;
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Activation Failed \n");
	} else {
		printf("Activated Succesfully !!\n");
	}
}

void displayLoans(){
	FILE *file = fopen("database/loans.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        return;
    }
    Loan loan;
    int found = 0;
    while (fscanf(file, "%d %s %s %s %f", &loan.id, loan.customer, loan.handler, loan.status, &loan.amount) != EOF) {
        if (strcmp(loan.handler, "NA") == 0) {
            printf("id: %d\ncustomer name: %s\nhandler employee: %s\nloan amount: %.2f\n", loan.id, loan.customer, loan.status, loan.amount);
            found = 1;
        }
    }
    fclose(file);
    if (!found) {
        printf("No loans with handler 'NA' found.\n");
    }
}

void assign_loan(int sock){
	displayLoans();
	Request req;
	char username[50];
	int id;
	printf("Enter the loan id that you want to assign: ");
	scanf("%d", &id);
	printf("Enter the employee name that will be assigned this loan: ");
	scanf("%s", username);
	strcpy(req.action, "LOAN_ASSIGN");
	req.user.id = id;
	strcpy(req.user.username, username);
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Loan was not assigned \n");
	} else {
		printf("Succesfull assigned loan !!\n");
	}
}

void logout_manager(int sock) {
	printf("\n");
	Request req;
	strcpy(req.action, "MANAGER_LOGOUT");
	send(sock, &req, sizeof(Request), 0);
}

void change_password_manager(int sock, User user) {
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

int displayFeedback(){
	FILE *file = fopen("database/feedbacks.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        return -1;
    }
	int count=0;
    Feedback feedback;
    int found = 0;
    while (fscanf(file, "%d %s %s", &feedback.id, feedback.givenby, feedback.status) != EOF) {
        fscanf(file, "%[^\n]", feedback.feedback);
        if (strcmp(feedback.status, "reviewed") != 0) {
			count++;
            printf("id: %d\nuser: %s\nstatus: %s\nfeedback: %s\n", feedback.id, feedback.givenby, feedback.status, feedback.feedback);
            found = 1;
        }
    }
    fclose(file);
    if (!found) {
        printf("No feedbacks with status other than 'reviewed' found.\n");
    }
	return count;
}

void review_feedback(int sock){
	int count;
	count = displayFeedback();
	int id;
	Request req;
	strcpy(req.action, "REVIEW_FEEDBACK");
	if(count!=0)
	{
		printf("Enter the feedback id that you have reviewed: ");
		scanf("%d", &id);
		req.user.id = id;
		send(sock, &req, sizeof(Request), 0);
	}	
}

void manager_menu(int sock, User user) {
	while (1) {
		printf("\n Manager Menu:\n");
		printf("1. Deactivate Customer Accounts\n");
		printf("2. Activate Customer Accounts\n");
		printf("3. Assign Loan Application Processes to Employees\n");
		printf("4. Review Customer Feedback\n");
		printf("5. Change Password\n");
		printf("6. Logout\n");
		printf("7. Exit\n");

		int choice;
		printf("Enter your choice: ");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			deactivate_customer_account(sock);
			break;
		case 2:
			activate_customer_account(sock);
			break;
		case 3:
			assign_loan(sock);
			break;
		case 4:
			review_feedback(sock);
			break;
		case 5:
			change_password_manager(sock, user);
			break;
		case 6:
			logout_manager(sock);
			return; 
		case 7:
			printf("Exiting...\n");
			exit(0); 
		default:
			printf("Invalid choice. Please try again.\n");
		}
	}

}