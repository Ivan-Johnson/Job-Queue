/*
 * Tmp/server.c
 *
 * This file demonstrates how sockets are used from the server side
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 1000

int main(int argc, char *argv[])
{
	int fail;

	int fdSock = socket(AF_INET, SOCK_STREAM, 0);
	if (fdSock < 0) {
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(7384);

	fail = bind(fdSock, (struct sockaddr *)&server, sizeof(server));
	if (fail) {
		return 2;
	}

	fail = listen(fdSock, 3);
	if (fail) {
		return 3;
	}

	puts("Waiting for incoming connections...");
	int c = sizeof(struct sockaddr_in);

	struct sockaddr_in client;
	while (1) {
		int fdClient = accept(fdSock, (void *)&client, (void *)&c);
		if (fdClient < 0) {
			return 4;
		}
		puts("Connected to client");

		//Receive a message from client
		char client_message[BUFLEN];
		while (1) {
			int read_size = recv(fdClient, client_message, BUFLEN, 0);
			if (read_size == 0) {
				puts("Client disconnected");
				break;
			} else if (read_size == -1) {
				perror("recv failed");
				return 5;
			}
			//Send the message back to client
			write(fdClient, client_message, read_size);
		}
	}
	assert(false);
}
