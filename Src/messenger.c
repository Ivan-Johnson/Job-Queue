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

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "messenger.h"
#include "server.h"

bool messengerSendJob(struct server server, struct job job)
{
	(void) server;
	(void) job;
	puts("messengerSendJob is not yet implemented");
	return false;
	//TODO:
	//return false if server not running
	//check status of fifo, initialize if dne
	//write job to fifo
}

__attribute__((noreturn)) static void* messengerReader(void *srvr)
{
	struct server server = *((struct server*) srvr);
	int fifo_read = openat(server.server, SFILE_FIFO, O_RDONLY | O_NONBLOCK);
	if (fifo_read == -1) {
		dprintf(server.err, "Could not open fifo for reading\n");

		// TODO: create some sort of serverUnmake(struct server)?
		close(server.err);
		close(server.log);
		close(server.fifo);
		// TODO: some sort of server_requestHalt?
		exit(1); // We use exit (rather than pthread_exit) to kill
			 // serverMain as well
	}

	while (1) {
		sleep(3);
		dprintf(server.log, "Reader Lives!\n");
	}
}

int messengerGetServer(const char *path, struct server *server)
{
	int status;
	status = serverInitialize(path, server);
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
                dprintf(server->err, "Failed to setsid: %s\n", strerror(errno));
                exit(1);
        }

	pthread_t unused;
	status = pthread_create(&unused, NULL, messengerReader, server);
	if (status) {
		dprintf(server->err, "Failed to start reader thread: %s\n",
			strerror(status));
		exit(1);
	}

	serverMain(server);
}
