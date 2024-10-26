#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "login.h"
#include "user.h"
#include "globals.h"
#include <semaphore.h>


// =========================================================================================================================================================
// USER RELATED FUNCTIONALITIES

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
void print_user(User user) {
    printf("\n");
    printf("User ID: %d\n", user.id);
    printf("Username: %s\n", user.username);
    printf("Password: %s\n", user.password);
    printf("Role: %d\n", user.role);
    printf("Account Balance: %.2f\n", user.account_balance);
    printf("\n");
}

FILE *openDataFile(char *flag) {
    FILE *file = fopen("database/users.txt", flag);
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    return file;
}

void printAllUser() {
    FILE *file = openDataFile("r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    User user;
    char line[150];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role, &user.account_balance);
        printf("%d %s %s %d %.2f\n", user.id, user.username, user.password, user.role, user.account_balance);
    }
    fclose(file);
}
void add_user(const char* username, char* password, int role) {
    printf("Adding user: %s\n", username);
    FILE *file = openDataFile("r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    int next_id = 1; 
    User user;
    while (fscanf(file, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role, &user.account_balance) == 5) {
        if (user.id >= next_id) {
            next_id = user.id + 1;
        }
    }
    fclose(file); 
    User new_user;
    strcpy(new_user.username, username);
    strcpy(new_user.password, password);
    new_user.role = role;
    new_user.id = next_id;
    new_user.account_balance = 0.0;
    file = openDataFile("a");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    fprintf(file, "%d %s %s %d %.2f\n", new_user.id, new_user.username, new_user.password, new_user.role, new_user.account_balance);
    printf("User added successfully!\n");
    fclose(file);
}

void modify_user(User user) {
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = openDataFile("r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    User temp_users[100]; 
    int user_found = 0;
    int index = 0;
    while (fscanf(file, "%d %s %s %d %f", &temp_users[index].id, 
                                        temp_users[index].username, 
                                        temp_users[index].password, 
                                        &temp_users[index].role, 
                                        &temp_users[index].account_balance) == 5) {
        if (temp_users[index].id == user.id) {
            temp_users[index] = user;
            user_found = 1;
        }
        index++;
    }
        fclose(file);
    if (!user_found) {
        printf("User not found.\n");
        sem_post(&file_sem);
        return;
    }
    file = openDataFile("w");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    for (int i = 0; i < index; i++) {
        fprintf(file, "%d %s %s %d %.2f\n", temp_users[i].id, 
                                            temp_users[i].username, 
                                            temp_users[i].password, 
                                            temp_users[i].role, 
                                            temp_users[i].account_balance);
    }
    fclose(file);
    printf("User modified successfully!\n");
    sem_post(&file_sem);
    sem_destroy(&file_sem);
}

void update_password(User current_user) 
{   sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = openDataFile("r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        return;
    }
    FILE *temp_file = fopen("database/temp_users.txt", "w");
    if (temp_file == NULL) {
        printf("Error! Can't open temporary file.\n");
        fclose(file);
        return;
    }
    User user;
    char line[150]; 
    int user_found = 0;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role,&user.account_balance);
        if (user.id == current_user.id) {\
            strcpy(user.password, current_user.password);
            user_found = 1;
        }
        fprintf(temp_file, "%d %s %s %d %.2f\n", user.id, user.username, user.password, user.role,user.account_balance);
    }
    fclose(file);
    fclose(temp_file);
    if (user_found) {
        remove("database/users.txt");
        rename("database/temp_users.txt", "database/users.txt");
        printf("Password updated successfully for user %d.\n", current_user.id);
    } else {
        printf("User with ID %d not found.\n", current_user.id);
        remove("database/temp_users.txt");
    }
    sem_destroy(&file_sem);
}

// =========================================================================================================================================================
// LOGIN RELATED FUNCTIONS

bool check_credentials(const char* username, const char *password){
    FILE *file = fopen("database/users.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    User user;
    char line[150];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id,user.username, user.password, &user.role,&user.account_balance);
        if (strcmp(username, user.username) == 0 && strcmp(password, user.password) == 0 && user.account_balance != -1) {
            current_user = user;
                printf("Logged In as %s \n", current_user.username);
                fclose(file);
                return true;   
        }
    }
    fclose(file);
    return false;
}

bool login(const char* username, const char* password) {
    if (check_credentials(username, password)) {
        printf("Login successful!\n");
        return true;
    } else {
            current_user.id = -1;
            printf("Invalid username or password.\n");
            return false;   
    }
} 

// =========================================================================================================================================================
// MONEY RELATED FUNCTIONALITIES

void addTransaction(Transaction trans) {
    FILE *file = fopen("database/transactions.txt", "a");  
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        exit(1);
    }
    fprintf(file, "%d %d %.2f %s %s %d\n", trans.from, trans.to, trans.amount, trans.from_username, trans.to_username, trans.self);
    fflush(file);
    fclose(file);
}
void deposit(User current_user, float amount) {
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = openDataFile("r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    FILE *temp_file = fopen("database/temp_users.txt", "w");
    if (temp_file == NULL) {
        printf("Error! Can't open temporary file.\n");
        fclose(file);
        sem_post(&file_sem);
        return;
    }
    User user;
    char line[150];  
    int user_found = 0;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role,&user.account_balance);
        if (user.id == current_user.id) {
            if (amount < 0 && user.account_balance < (-1.00 * amount)) {
                printf("Error doing transaction due to insufficient funds.\n");
                fclose(file);
                fclose(temp_file);
                remove("temp_users.txt");
                sem_post(&file_sem);
                return;
            }
            user.account_balance += amount; 
            user_found = 1;
        }
        fprintf(temp_file, "%d %s %s %d %.2f\n", user.id, user.username, user.password, user.role, user.account_balance);
    }
    fclose(file);
    fclose(temp_file);
    if (user_found) {
        remove("database/users.txt"); 
        rename("database/temp_users.txt", "database/users.txt"); 
        printf("Transaction successful. New balance for user %d: %.2f\n", current_user.id, user.account_balance);
    } else {
        printf("User with ID %d not found.\n", current_user.id);
        remove("temp_users.txt");
    }
    sem_post(&file_sem);
    sem_destroy(&file_sem);
}

User get_balance(User current_user) {
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/users.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        exit(1);
    }
    User user;
    char line[150];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s %d %f", &user.id, user.username, user.password, &user.role,&user.account_balance);
        if (user.id == current_user.id) {
            fclose(file);
            sem_post(&file_sem);
            return user;
        }
    }
    fclose(file);
    printf("User with ID %d not found.\n", current_user.id);
    User empty_user = {0};
    sem_post(&file_sem);
    sem_destroy(&file_sem);
    return empty_user;
}

// =========================================================================================================================================================
// LOAN RELATED FUNCTIONALITIES

void loanapplication(User curr_user, float amount){
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/loans.txt","r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    Loan loan;
    int next_id = 1;
    while(fscanf(file,"%d %d %s %s %s %f", &loan.id, &loan.customer_id, loan.customer, loan.handler, loan.status, &loan.amount)==5)
    {
        if(loan.id>=next_id){
            next_id = loan.id+1;
        }
    }
    fclose(file);
    Loan new_loan;
    new_loan.id = next_id;
    new_loan.customer_id = curr_user.id;
    strcpy(new_loan.customer,curr_user.username);
    strcpy(new_loan.handler,"NA");
    strcpy(new_loan.status,"Pending");
    new_loan.amount = amount;
    file = fopen("database/loans.txt","a");
    if (file == NULL) {
        printf("Error! Can't open file for writing.\n");
        sem_post(&file_sem);
        return;
    }
    fprintf(file, "%d %d %s %s %s %.2f\n", new_loan.id, new_loan.customer_id, new_loan.customer, new_loan.handler, new_loan.status, new_loan.amount);
    fclose(file);
    sem_post(&file_sem);
    printf("Loan application submitted successfully! Loan ID: %d\n", new_loan.id);
    sem_destroy(&file_sem);
}

void loanAssign(int id, char *username)
{  
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/loans.txt","r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    Loan loan;
    int found = 0;
    FILE *temp_file = fopen("database/temp_loans.txt", "w");
    if (temp_file == NULL) {
        printf("Error! Can't open temp file.\n");
        fclose(file);
        sem_post(&file_sem);
        return;
    }
    while (fscanf(file, "%d %d %s %s %s %f", &loan.id, &loan.customer_id, loan.customer, loan.handler, loan.status, &loan.amount) != EOF) {
        if (loan.id == id) {
            strcpy(loan.handler, username);
            found = 1;
        }
        fprintf(temp_file, "%d %d %s %s %s %.2f\n", loan.id, loan.customer_id, loan.customer, loan.handler, loan.status, loan.amount);
    }
    fclose(file);
    fclose(temp_file);
    if (found) {
        remove("database/loans.txt");
        rename("database/temp_loans.txt", "database/loans.txt");
        printf("Loan ID %d has been successfully assigned to employee %s.\n", id, username);
    } else {
        remove("database/temp_loans.txt");
        printf("Loan ID %d not found.\n", id);
    }
    sem_post(&file_sem);
    sem_destroy(&file_sem);
}

void approveRejectLoan(int id, char *status){
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/loans.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    Loan loan;
    int found = 0;
    FILE *temp_file = fopen("database/temp_loans.txt", "w");
    if (temp_file == NULL) {
        printf("Error! Can't open temp file.\n");
        fclose(file);
        sem_post(&file_sem);
        return;
    }
    while (fscanf(file, "%d %d %s %s %s %f", &loan.id, &loan.customer_id,loan.customer, loan.handler, loan.status, &loan.amount) != EOF) {
       if (loan.id == id) {
            strcpy(loan.status, status);
            found = 1;
            User user;
            user = getUser(loan.customer_id);
            if(strcmp(status,"approved")==0)
            {
                deposit(user,loan.amount);
                Transaction trans;
                trans.to = user.id;
                trans.from = 0;
                trans.amount = loan.amount;
                trans.self = 0;
                strcpy(trans.from_username, "BANK");
                strcpy(trans.to_username,user.username);
                addTransaction(trans);
            }
        }
        fprintf(temp_file, "%d %d %s %s %s %.2f\n", loan.id, loan.customer_id,loan.customer, loan.handler, loan.status, loan.amount);
    }
    fclose(file);
    fclose(temp_file);
    if (found) {
        remove("database/loans.txt");
        rename("database/temp_loans.txt", "database/loans.txt");
        printf("Loan ID %d status has been changed to %s.\n", id, status);
    } else {
        remove("database/temp_loans.txt");
        printf("Loan ID %d not found.\n", id);
    }
    sem_post(&file_sem);
    sem_destroy(&file_sem);    
}

// =========================================================================================================================================================
//FEEDBACK RELATED FUNCTIONS

void addFeedback(User curr_user, char *feedback){
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/feedbacks.txt", "a");
    printf("%s",feedback);
    if (file == NULL) {
        printf("Error! Can't open feedbacks.txt file.\n");
        exit(1);
        sem_post(&file_sem);
    }
    FILE *readFile = fopen("database/feedbacks.txt", "r");
    if (readFile == NULL) {
        printf("Error! Can't open feedbacks.txt file for reading.\n");
        exit(1);
        sem_post(&file_sem);
    }
    Feedback feed;
    int next_id = 1;
    while (fscanf(readFile, "%d %s %s %[^\n]", &feed.id, feed.givenby, feed.status, feed.feedback) == 4) {
        next_id = feed.id + 1;
    }
    fclose(readFile);
    const char* status = "not_reviewed";
    fprintf(file, "%d %s %s %s\n", next_id, curr_user.username, status,feedback);
    fclose(file);
    sem_post(&file_sem);
    printf("Feedback added successfully!\n");
    sem_destroy(&file_sem);
}

void feedbackReview(int id)
{
    sem_t file_sem;
    sem_init(&file_sem, 0, 1);
    sem_wait(&file_sem);
    FILE *file = fopen("database/feedbacks.txt", "r");
    if (file == NULL) {
        printf("Error! Can't open file.\n");
        sem_post(&file_sem);
        return;
    }
    Feedback feedback;
    int found = 0;
    FILE *temp_file = fopen("database/temp_feedbacks.txt", "w");
    if (temp_file == NULL) {
        printf("Error! Can't open temp file.\n");
        fclose(file);
        sem_post(&file_sem);
        return;
    }
    while (fscanf(file, "%d %s %s %[^\n]\n", &feedback.id, feedback.givenby, feedback.status, feedback.feedback) != EOF) {
       if (feedback.id == id) {
            strcpy(feedback.status, "reviewed");
            found = 1;
        }
        fprintf(temp_file, "%d %s %s %s\n", feedback.id, feedback.givenby, feedback.status, feedback.feedback);
    }
    fclose(file);
    fclose(temp_file);
    if (found) {
        remove("database/feedbacks.txt");
        rename("database/temp_feedbacks.txt", "database/feedbacks.txt");
        printf("Feedback ID %d status has been changed to 'reviewed'.\n", id);
    } else {
        remove("database/temp_feedbacks.txt");
        printf("Feedback ID %d not found.\n", id);
    }
    sem_post(&file_sem);
    sem_destroy(&file_sem);
}
// =========================================================================================================================================================




