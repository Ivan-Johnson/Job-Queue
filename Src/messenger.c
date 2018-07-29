/*
 * Src/messenger.c
 *
 * Implements Src/messenger.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

//for dprintf
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "job.h"
#include "messenger.h"
#include "server.h"

int messengerSendJob(struct server server, struct job job)
{
	char buf[PIPE_BUF];
	ssize_t len = serializeJob(job, buf, PIPE_BUF);
	if (len == -1) {
		puts("The given job does not fit in our buffer");
		return 1;
	}
	assert(len >= 0);
	ssize_t s = (ssize_t) write(server.fifo, buf, (size_t) len);
	return s != len;
}

__attribute__((noreturn)) static void* messengerReader(void *srvr)
{
	struct server server = *((struct server*) srvr);
	fprintf(server.log, "Messenger is initializing\n");
	fflush(server.log);
	int fifo_read = openat(server.server, SFILE_FIFO, O_RDONLY | O_NONBLOCK);
	if (fifo_read == -1) {
		fprintf(server.err, "Could not open fifo for reading\n");

		serverClose(server);

		serverShutdown(false);
		pthread_exit(NULL);
	}
	fprintf(server.log, "Messenger successfully opened the fifo for reading\n");
	fflush(server.log);

	while (1) {
		sleep(1); //TODO use pselect or something?
		char buf[PIPE_BUF];
		ssize_t s = read(fifo_read, buf, PIPE_BUF);
		if (s <= 0) {
			continue;
		}
		if (buf[s] != '\0') {
			fprintf(server.err,
				"Reader received an invalid string\n");
			fflush(server.err);
			serverShutdown(false);
			pthread_exit(NULL);
		}
		struct job job;
		unserializeJob(&job, buf, (size_t) s);
		int status = serverAddJob(job, false);
		if (status) {
			fprintf(server.err, "Error when scheduling job: %s\n",
				job.cmd);
			fflush(server.err);
		} else {
			fprintf(server.log, "Scheduled job: %s\n", job.cmd);
		}
	}
}

int messengerGetServer(const char *path, struct server *server)
{
	int status;
	status = serverOpen(path, server);
	if (status == SIC_running) {
		puts("Server is already running");
		return 0;
	} else if (status == SIC_failed) {
		puts("SIC_failed");
		return 1;
	} else if (status != SIC_initialized) {
		puts("Invalid SIC code?");
		return 1;
	}

	int pid = fork();
	if (pid == -1) {
		puts("Failed to fork a server");
		// TODO: close fd's in server
		return 1;
	} else if (pid != 0) {
		// Origional process immediately returns successfully
		puts("Successfully forked a server");
		return 0;
	}

	status = setsid();
	if (status == -1) {
		fprintf(server->err, "Failed to setsid: %s\n", strerror(errno));
		exit(1);
	}

	pthread_t unused;
	status = pthread_create(&unused, NULL, messengerReader, server);
	if (status) {
		fprintf(server->err, "Failed to start reader thread: %s\n",
			strerror(status));
		exit(1);
	}

	serverMain(server);
}
