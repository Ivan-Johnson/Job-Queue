/*
 * Tmp/client.c
 *
 * This file demonstrates how sockets are used from the client side
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define BUFLEN 1000

int main(int argc, char *argv[])
{
	int fail;

	int fdSock = socket(AF_INET, SOCK_STREAM, 0);
	if (fdSock < 0) {
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(7384);

	fail = connect(fdSock, (void *)&addr, sizeof(addr));
	if (fail) {
		return 2;
	}

	puts("Connected\n");

	while (1) {
		char message[BUFLEN];
		printf("Enter message: ");
		char *pass = fgets(message, BUFLEN, stdin);
		if (!pass) {
			return 5;
		}

		int i = 0;
		while (pass[i] != '\0') {
			if (pass[i] == '\n') {
				pass[i] = '\0';
				break;
			}
			i++;
		}

		if (strlen(message) == 0) {
			return 0;
		}

		printf("Sending message \"%s\"\n", message);
		fail = send(fdSock, message, strlen(message) + 1, 0);
		if (fail <= 0) {
			return 3;
		}

		puts("Sent successfully");

		fail = recv(fdSock, message, BUFLEN, 0);
		if (fail <= 0) {
			return 4;
		}

		puts("Server reply: ");
		puts(message);
	}
	assert(false);
}
