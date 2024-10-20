#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "user.h"
#include "login.h"
#include "globals.h"

User getUserClient(int id) {
    FILE *file = fopen("database/users.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    User user;
    char line[150]; 
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role, &user.account_balance);
        if (user.id == id) {
            fclose(file);
            return user;
        }
    }
    fclose(file);
    user.id = -1;  
    return user;
}

void logout_customer(int sock) {
	printf("\n");
	Request req;
	strcpy(req.action, "CUSTOMER_LOGOUT");
	send(sock, &req, sizeof(Request), 0);
}

void get_account_balance(int sock, User user) {
	Request req;
	strcpy(req.action, "GET_BALANCE");
	req.user = user;
	send(sock, &req, sizeof(Request), 0);
	ssize_t bytes_received = recv(sock, &user, sizeof(user), 0);
	if (bytes_received < 0) {
		perror("Receive failed");
		return;
	} else if (bytes_received == 0) {
		printf("Connection closed by peer\n");
		return;
	}
	printf("\n Current Account Balance: %.4f \n", user.account_balance);
}

void getLoandetails(const char *username){
	FILE *file = fopen("database/loans.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open loans file.\n");
        return;
    }
    Loan loan;
    int found = 0;
    while (fscanf(file, "%d %s %s %s %f", &loan.id, loan.customer, loan.handler, loan.status, &loan.amount) == 5) {
        if (strcmp(loan.customer, username) == 0) {
            found = 1;
            printf("Loan ID: %d\n", loan.id);
            printf("Customer: %s\n", loan.customer);
            printf("Handler Employee : %s\n", loan.handler);
            printf("Status: %s\n", loan.status);
            printf("Amount: %.2f\n\n", loan.amount);
        }
    }
    if (!found) {
        printf("No loans found for user: %s\n", username);
    }
    fclose(file);
}

void apply_for_loan(int sock, User user){
	float loan_amount;
	int n;
	printf("Enter your choice\n");
	printf("1-Loan application\n");
	printf("2-Display Loan details\n");
	scanf("%d",&n);
	if(n==1){
		printf("Enter the amount of your loan\n");
		scanf("%f",&loan_amount);
		user = getUserClient(user.id);
		Request req;
		req.amount = loan_amount;
		req.user = user;
		strcpy(req.action, "LOAN");
		ssize_t bytes = send(sock, &req, sizeof(Request), 0);
		if (bytes < 0) {
			printf("Loan application Failed \n");
		} else {
			printf("Loan application Succesfull !!\n");
		}
		getLoandetails(user.username);
	}
	else if(n==2){
		getLoandetails(user.username);
	}
	else {
		printf("Invalid choice\n");
		return;
	}
}

void add_feedback_customer(int sock, User user){
	char feedback[2048];
	printf("Please give your feedback(in one single line): ");
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	fgets(feedback, 2048, stdin);
	Request req;
	req.user = user;
	strcpy(req.feedback, feedback);
	strcpy(req.action, "FEEDBACK");
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Feedback not added\n");
	} else {
		printf("Succesfull added feedback !!\n");
	}
}

void deposit_money(int sock, User user, bool flag) {
	float deposit_amount;
	if(!flag){
		printf("Enter the amount to withdraw: ");
	} 
	else printf("Enter the amount to deposit: ");
	scanf("%f", &deposit_amount);
	user = getUserClient(user.id);
	if (!flag) {
		if(deposit_amount > user.account_balance){
			printf("\n You have insuffienct funds \n");
			return;
		}
		deposit_amount = -1.00 * deposit_amount;
	}
	Request req;
	req.amount = deposit_amount;
	req.user = user;
	strcpy(req.action, "DEPOSIT");
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Transaction Failed \n");
	} else {
		printf("Transaction Succesfull !!\n");
	}
	if(!flag) printf("Updated account balance is : %f",user.account_balance);
}

void transfer_funds(int sock, User user) {
	int id;
	printf("Enter the Account ID for Transfer: ");
	scanf("%d", &id);
	float amount;
	printf("Enter the amount to send: ");
	scanf("%f", &amount);
	Request req;
	req.transfer_id = id;
	req.user = user;
	req.amount = amount;
	strcpy(req.action, "TRANSFER_FUNDS");
	ssize_t bytes = send(sock, &req, sizeof(Request), 0);
	if (bytes < 0) {
		printf("Transaction Failed \n");
	}

	char err[50];
	recv(sock, &err, sizeof(err), 0);


	if (strcmp(err, "insuffienct funds") == 0) printf("\n You have %s\n", err);

}

void change_password(int sock, User user) {
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

void view_transaction_history(int sock, User user) {
    FILE *file = fopen("database/transactions.txt", "r");  
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    Transaction trans;
    char line[150];  
    int i = 20;      
    while (i && fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %d %f %s %s", &trans.from, &trans.to, &trans.amount, trans.from_username, trans.to_username);
        if (trans.from == user.id || trans.to == user.id) {
            i--;
            if (trans.to == user.id) {
                if (trans.amount < 0) {
                    printf("You have Withdrawn (%.4f)\n", -trans.amount); 
                } else {
                    printf("You have Deposited (%.4f)\n", trans.amount);
                }
            } else {
                if (trans.amount < 0) {
                    printf("You have Transferred (%.4f) to %s\n", -trans.amount, trans.to_username);
                } else {
                    printf("You have Received (%.4f) from %s\n", trans.amount, trans.to_username);
                }
            }
        }
    }
	fclose(file);
}

void customer_menu(int sock, User user) {
	while (1) {
		printf("\nCustomer Menu:\n");
		printf("1. View Account Balance\n");
		printf("2. Deposit Money\n");
		printf("3. Withdraw Money\n");
		printf("4. Transfer Funds\n");
		printf("5. Apply for a Loan\n");
		printf("6. Change Password\n");
		printf("7. Add Feedback\n");
		printf("8. View Transaction History\n");
		printf("9. Logout\n");
		printf("10. Exit\n");

		int choice;
		printf("Enter your choice: ");
		scanf("%d", &choice);

		switch (choice) {
		case 1:
			get_account_balance(sock, user);
			break;
		case 2:
			deposit_money(sock, user, 1);
			break;
		case 3:
			deposit_money(sock, user, 0);
			break;
		case 4:
			transfer_funds(sock, user);
			break;
		case 5:
			apply_for_loan(sock, user);
			break;
		case 6:
			change_password(sock, user);
			break;
		case 7:
			add_feedback_customer(sock, user);
			break;
		case 8:
			printf("\n");
			view_transaction_history(sock, user);
			printf("\n");
			break;
		case 9:
			logout_customer(sock);
			return;
		case 10:
			printf("Exiting...\n");
			exit(0); 
		default:
			printf("Invalid choice. Please try again.\n");
		}
	}
}


