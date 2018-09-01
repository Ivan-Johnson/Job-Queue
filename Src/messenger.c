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

int messengerSendJob(int serverdir, struct job job)
{
	int fifo = openat(serverdir, SFILE_FIFO,
			O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (fifo < 0) {
		if (errno == ENXIO || errno == ENOENT) {
			puts("The server is not running");
		}
		return 1;
	}

	char buf[PIPE_BUF];
	ssize_t len = serializeJob(job, buf, PIPE_BUF);
	if (len == -1) {
		puts("The given command is too long");
		return 1;
	}
	assert(len >= 0);
	puts("Sending job");
	ssize_t s = (ssize_t) write(fifo, buf, (size_t) len);
	if (s == -1) {
		printf("Failed to send: %s\n", strerror(errno));
		return 1;
	} else if (s != len) {
		//This should never happen, because we're limiting our message
		//to PIPE_BUF bytes
		puts("Sent a partial message??");
		return 1;
	}
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

static void processFIFO(struct server server, int fifo)
{
	char buf[PIPE_BUF];
	// number of bytes in buf that are currently in use
	size_t bufused = 0;
	while (1) {
		ssize_t s = read(fifo, buf + bufused, PIPE_BUF - bufused);
		if (s < 0) {
			if (errno == EINTR) {
				return;
			}
			s = 0;
		}
		bufused += (size_t) s;
		if (bufused <= 0) {
			return;
		}
		struct job job;
		char *next;
		int fail = unserializeJob(&job, buf, bufused, &next);
		if (fail) {
			fprintf(server.err, "Unserialization failed\n");
			fflush(server.err);
			return;
		}

		fail = processJob(job);
		if (fail) {
			fprintf(server.err, "Error when scheduling job: %s\n",
				job.argv[0]);
			fflush(server.err);
		} else {
			fprintf(server.log, "Scheduled job: %s\n", job.argv[0]);
			fflush(server.log);
		}
		freeUnserializedJob(job);

		// (buf) (next) (buf+bufused)
		//
		// copies the values that are currently in the range
		// [next, buf+bufunused] to the front of buf.
		bufused -= (size_t) (next - buf);
		memmove(buf, next, bufused);
	}
}

__attribute__((noreturn)) static void* messengerReader(void *srvr)
{
	struct server server = *((struct server *) srvr);
	fprintf(server.log, "Messenger is initializing\n");
	fflush(server.log);
	int fifo_read = openat(server.server, SFILE_FIFO,
			O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fifo_read == -1) {
		fprintf(server.err, "Could not open fifo for reading\n");

		serverClose(server);

		serverShutdown(false);
		pthread_exit(NULL);
	}
	fprintf(server.log,
		"Messenger successfully opened the fifo for reading\n");
	fflush(server.log);

	while (1) {
		sleep(1); //TODO use pselect or something?
		processFIFO(server, fifo_read);
	}
}

int messengerLaunchServer(int fd, unsigned int numSlots)
{
	if (numSlots == 0) {
		numSlots = 1;
	}
	int status;
	struct server server;
	status = openServer(fd, &server, numSlots);
	if (status) {
		return 1;
	}

	int pid = fork();
	if (pid == -1) {
		puts("Failed to fork a server");
		serverClose(server);
		return 1;
	} else if (pid != 0) {
		// Origional process immediately returns successfully
		puts("Successfully forked a server");
		return 0;
	}

	status = setsid();
	if (status == -1) {
		fprintf(server.err, "Failed to setsid: %s\n", strerror(errno));
		exit(1);
	}

	pthread_t unused;
	status = pthread_create(&unused, NULL, messengerReader, (void*) &server);
	if (status) {
		fprintf(server.err, "Failed to start reader thread: %s\n",
			strerror(status));
		exit(1);
	}

	serverMain((void*) &server);
}
