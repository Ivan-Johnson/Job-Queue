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

struct job {
	// index 0 is the command we run. Indicies [1, argc] are arguments
	char *cmd;
};

//a fifo file that exists in serverdir. As long as the server is running, this
//fifo will be opened for reading.
#define SFILE_FIFO "fifo"

void serverMain(int serverfd);
bool serverAddJob(struct job job);

#endif
