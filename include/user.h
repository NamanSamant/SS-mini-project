#ifndef USER_H
#define USER_H

typedef struct {
    int id;                 
    char username[50];      
    char password[50];      
    int role;// Role of the user (1: customer, 2: employee, 3: manager, 4: admin)
    float account_balance;
} User;

typedef struct {
    User user;
    char action[50];
    float amount;
    int transfer_id;
    char feedback[2048];
    char status[20];
} Request;

typedef struct {
    int from;
    int to;
    float amount;
    char from_username[50];
    char to_username[50];
} Transaction;

typedef struct {
    int id;
    char customer[50];
    char handler[50];
    char status[20];
    float amount;
} Loan;

typedef struct {
    int id;
    char givenby[50];
    char status[50];
    char feedback[2048] ;
} Feedback;

#endif 
