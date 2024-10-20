#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "user.h"
#include "login.h"
#include "admin.h"
#include "globals.h"
#include "banking.h"
#include "customer.h"
#include "employee.h"
#include "manager.h"

#define PORT 8080
User current_user;

int main() {
	int sock;
	struct sockaddr_in server_addr;
	char username[50], password[50];
	User received_user;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Could not create socket");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connection failed");
		return 1;
	}
	while (1) {
		printf("Enter username: ");
		scanf("%s", username);
		printf("Enter password: ");
		scanf("%s", password);
		send(sock, username, sizeof(username), 0);
		send(sock, password, sizeof(password), 0);
		if (recv(sock, &received_user, sizeof(received_user), 0) > 0) {
			if (received_user.id != -1) {
				current_user = received_user;
				printf("Logged in as: %s\n", received_user.username);
				if (received_user.role == 4) {
					admin_menu(sock, received_user);
					continue;
				} else if (received_user.role == 3) {
					manager_menu(sock, received_user);
				} else if (received_user.role == 2) {
					employee_menu(sock, received_user);
				} else {
					customer_menu(sock, received_user);
					continue;
				}
			} else {
					printf("Login failed!\n");
				}	
		} else {
			printf("Failed to receive response from server.\n");
		}
	}
	close(sock);
	return 0;
}
