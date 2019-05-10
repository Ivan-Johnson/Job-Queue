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

void serverMain(void *srvr) __attribute__((noreturn));
int serverAddJob(struct job job);
int serverShutdown(bool killRunning);

//Create an empty server directory at the specified path and returns a file
//descriptor to it. If the specified directory already exists, and it has the
//correct permissions, simply return a file descriptor to it.
//
//Returns -1 on failure, and returns a non-negative file descriptor to a
//directory on success.
int getServerDir(const char *path);

// Given a file descriptor of the server's main directory, returns the port
// number that the server is on. Returns a negative number on error.
int serverGetPort(int dirFD);

//TODO change fdServer to just be a path, then remove getServerDir.
//
//numSlots == 0: use an implementation defined number of slots
int serverForkNew(int fdServer, unsigned int numSlots, unsigned int port);

#endif
