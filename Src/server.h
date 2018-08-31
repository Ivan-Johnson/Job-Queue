#ifndef SERVER_H
#define SERVER_H
/*
 * Src/server.h
 *
 * The server is responsible for tracking and running jobs
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include "job.h"

//a fifo file that exists in serverdir. As long as the server is running, this
//fifo will be opened for reading.
#define SFILE_FIFO "fifo"
#define SERVER_DIR_PERMS 0700

struct server {
	int server; // fd of the main server directory
	int fifo;   // fd of the fifo file used to receive requests (RD_ONLY)
	FILE *log;  // A file to use in place of the server's stdout
	FILE *err;  // A file to use in place of the server's stderr

	unsigned int numSlots;

	// buffer for the EXCLUSIVE use of the server thread
	// guaranteed have a length of at least numSlots
	unsigned int *slotBuff;
};

void serverMain(void *srvr) __attribute__((noreturn));
int serverAddJob(struct job job);
int serverShutdown(bool killRunning);

/*
 * Attempts to initialize s in preparation for launching a server.
 *
 * Returns 0 on success, nonzero on failure
 *
 * numSlots > 0
 */
int openServer(int dirFD, struct server *s, unsigned int numSlots);

void serverClose(struct server s);

//Create an empty server directory at the specified path and returns a file
//descriptor to it. If the specified directory already exists, and it has the
//correct permissions, simply return a file descriptor to it.
//
//Returns -1 on failure, and returns a non-negative file descriptor to a
//directory on success.
int getServerDir(const char *path);

#endif
