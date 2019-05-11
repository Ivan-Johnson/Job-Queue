/*
 * Src/messenger.c
 *
 * Implements Src/messenger.h
 *
 * Copyright(C) 2018-2019, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

//for dprintf
#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "job.h"
#include "messenger.h"
#include "server.h"

#define MAX_PENDING_CONNECTIONS 20
#define MAX_JOB_LENGTH 10000

/*
 * Returns the file descriptor of a socket that has been bound to the given
 * port.
 *
 * A negative return value indicates an error.
 */
static int createSocketFD(int port, bool client)
{
	int fail;

	int fdSock = socket(AF_INET, SOCK_STREAM, 0);
	if (fdSock < 0) {
		return -1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	if (client) { // TODO: test
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
	} else {
		server.sin_addr.s_addr = INADDR_ANY;
	}
	assert(0 < port && port <= UINT16_MAX);
	server.sin_port = htons((uint16_t) port);

	if (client) {
		fail = connect(fdSock, (void *)&server, sizeof(server));
		if (fail) {
			return -4;
		}
	} else {
		fail = bind(fdSock, (struct sockaddr *)&server, sizeof(server));
		if (fail) {
			return -2;
		}

		fail = listen(fdSock, MAX_PENDING_CONNECTIONS);
		if (fail) {
			return -3;
		}
	}
	return fdSock;
}

int messengerSendJob(int serverdir, struct job job)
{
	int port = serverGetPort(serverdir);
	assert(port > 0);

	int fdSock = createSocketFD(port, true);
	assert(fdSock >= 0);

	char buf[MAX_JOB_LENGTH];
	ssize_t len = serializeJob(job, buf, MAX_JOB_LENGTH);
	if (len < 0) {
		assert(len == -1);

		puts("The given command is too long");
		return 1;
	}

	ssize_t s = (ssize_t) write(fdSock, buf, (size_t)len);
	if (s == -1) {
		printf("Failed to send: %s\n", strerror(errno));
		return 1;
	}
	assert(s == len); //TODO: this is false; see man page write(2).
	return 0;
}

static int processJob(struct job job)
{
	struct job jobTmp = job;
	int fail = cloneJob(&job, jobTmp);
	if (fail) {
		return 1;
	}

	return serverAddJob(job);
}

static void processFD(int fd, FILE *fLog, FILE *fErr)
{
	char buf[MAX_JOB_LENGTH];
	// number of bytes in buf that are currently in use
	size_t bufused = 0;
	while (1) {
		ssize_t s = read(fd, buf + bufused, MAX_JOB_LENGTH - bufused);
		if (s < 0) {
			if (errno == EINTR) {
				return;
			}
			s = 0;
		}
		bufused += (size_t)s;
		if (bufused <= 0) {
			return;
		}
		struct job job;
		char *next;
		int fail = unserializeJob(&job, buf, bufused, &next);
		if (fail) {
			fprintf(fErr, "Unserialization failed\n");
			fflush(fErr);
			return;
		}

		fail = processJob(job);
		if (fail) {
			fprintf(fErr, "Error when scheduling job: %s\n",
				job.argv[0]);
			fflush(fErr);
		} else {
			fprintf(fLog, "Scheduled job: %s\n", job.argv[0]);
			fflush(fLog);
		}
		freeUnserializedJob(job);

		// (buf) (next) (buf+bufused)
		//
		// copies the values that are currently in the range
		// [next, buf+bufunused] to the front of buf.
		bufused -= (size_t)(next - buf);
		memmove(buf, next, bufused);
	}
}

__attribute__((noreturn))
void *messengerReader(void *srvr)
{
	struct messengerReaderArgs args = *((struct messengerReaderArgs *)srvr);
	fprintf(args.log, "Messenger is initializing\n");
	fflush(args.log);

	int port = serverGetPort(args.server);
	assert(port > 0);
	fprintf(args.log, "Messenger given port %d\n", port);

	int fdSock = createSocketFD(port, false);
	if (fdSock < 0) {
		printf("Error while creating socket: %s\n", strerror(errno));
		exit(1);
	}

	fprintf(args.log, "Server is now listening for incoming connections\n");
	fflush(args.log);

	struct sockaddr_in client;
	int client_size = sizeof(client);
	while (1) {
		int fdClient = accept(fdSock,
				(void *)&client, (void *)&client_size);
		assert(fdClient >= 0);
		fprintf(args.log, "Messenger received a new connection\n");

		processFD(fdClient, args.log, args.err);
	}
}
