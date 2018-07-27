/*
 * Src/messenger.c
 *
 * Implements Src/messenger.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdio.h>
#include <stdlib.h>
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
		// TODO: close fd's in server
                return 1;
        } else if (pid != 0) {
		// Origional process immediately returns successfully
                return 0;
        }

	// As a child, we've inherited copies of all file descriptors, so we can
	// use server without having to worry about the parent closing file
	// descriptors
	struct server svr = *server;

        status = setsid();
        if (status == -1) {
                puts("BAD THINGS ARE HAPPENING!");
                exit(1);
        }
        serverMain(svr); //never returns
}
