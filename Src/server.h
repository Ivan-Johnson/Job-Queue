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

//a fifo file that exists in serverdir. As long as the server is running, this
//fifo will be opened for reading.
#define SFILE_FIFO "fifo"
#define SERVER_DIR_PERMS S_IRWXU

struct job {
	// index 0 is the command we run. Indicies [1, argc] are arguments
	char *cmd;
};

struct server {
	int server; // fd of the main server directory
	int fifo;   // fd of the fifo file used to receive requests (WR_ONLY)
	FILE *log;  // A file to use in place of the server's stdout
	FILE *err;  // A file to use in place of the server's stderr
};

void serverMain(void *srvr) __attribute__((noreturn));
int serverAddJob(struct job job, bool isPriority);
int serverShutdown(bool killRunning);

enum serverInitCode {
	SIC_initialized = 0, // success
	SIC_failed,          // generic failure code
	SIC_running,         // failed because a server is already using path
};
/*
 * Attempts to initialize s in preparation for launching a server.
 *
 * s can be used to launch a server if and only if SS_initialized is
 * returned.
 */
enum serverInitCode serverOpen(const char *path, struct server *s);

void serverClose(struct server s);

#endif
