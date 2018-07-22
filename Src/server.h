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

enum serverState {
	unknown, //* unexpected state; should be impossible
	invalid, //* e.g. dir does not exist, or has bad permissions
	stopped, //* Server dir is valid, but server is not up
	running, //* everything checks out
	error    //* an error occured when checking the state
};

//a fifo file that exists in serverdir. As long as the server is running, this
//fifo will be opened for reading.
#define SFILE_FIFO "fifo"

void serverMain(char *serverDir);
bool serverAddJob(struct job job);

enum serverState serverStatus(const char *serverDir);

#endif
