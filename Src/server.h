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
	dne,     //* The specified dir does not exist
	invalid, //* e.g. dir exists, but with bad permissions
	stopped, //* Server dir is valid, but process is not valid (note that
	         //  the process could exist but be invalid if the computer
	         //  reuses PIDs, e.g. it has rebooted)
	running, //* everything checks out
	unknown  //* unexpected state; should be impossible
};

void serverMain(char *serverDir);
bool serverAddJob(struct job job);

enum serverState serverStatus(char *serverDir);

#endif
