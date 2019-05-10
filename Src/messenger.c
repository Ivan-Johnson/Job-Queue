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
	ssize_t s = (ssize_t) write(fifo, buf, (size_t)len);
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

static void processFIFO(int fifo, FILE *fLog, FILE *fErr)
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
		// [next, buf+bufunused] to the front of buf.
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
	(void) port; // TODO


	int fifo_read = openat(args.server, SFILE_FIFO,
			       O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fifo_read == -1) {
		fprintf(args.err, "Could not open fifo for reading\n");

		serverShutdown(false);
		pthread_exit(NULL);
	}
	fprintf(args.log,
		"Messenger successfully opened the fifo for reading\n");


	fflush(args.log);
	while (1) {
		sleep(1);	//TODO use pselect or something?
		processFIFO(fifo_read, args.log, args.err);
	}
}
