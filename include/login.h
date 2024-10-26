#include <stdbool.h>
#ifndef LOGIN_H
#define LOGIN_H
#include "user.h"


// Function prototypes
void addTransaction(Transaction trans);
User getUser(int id);
bool login(const char* username, const char* password);
void add_user(const char* username, char *password, int role);
void printAllUser();
void modify_user(User user);
void loanapplication(User curr_user, float amount);
void addFeedback(User curr_user, char *feedback);
void deposit(User user, float amount);
User get_balance(User user);
void update_password(User user);
void loanAssign(int id, char *username);
void feedbackReview(int id);
void approveRejectLoan(int id, char *status);
#endif // LOGIN_H
