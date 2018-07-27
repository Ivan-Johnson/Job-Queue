/*
 * Src/messenger.c
 *
 * Implements Src/messenger.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "messenger.h"

bool messengerSendJob(struct server server, struct job job)
{
	(void) server;
	(void) job;
	exit(1);
	//TODO:
	//return false if server not running
	//check status of fifo, initialize if dne
	//write job to fifo
}

#include <stdio.h>
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

	//server is initialized properly; we just need to launch the server
	puts("Just need to launch server now.");
	return 255;
}
