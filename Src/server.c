/*
 * Src/server.c
 *
 * implements Src/server.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
//for fstatat
#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "messenger.h"
#include "server.h"

enum serverState serverStatus(int serverfd)
{
	// TODO are there any remaining race conditions? (even if not harmful)
	struct stat st;
	int status;
	status = fstatat(serverfd, ".", &st, 0);
	if (status != 0) {
		return invalid;
	}
	if (st.st_uid != geteuid()) { //user doesn't own the server
		return invalid;
	}
	mode_t mode = st.st_mode;
	//only look at user/group/other's read/write/execute perms
	mode = mode & (S_IRWXU | S_IRWXG | S_IRWXO);
	if (mode != S_IRWXU) {
		return invalid;
	}

	int fd = openat(serverfd, SFILE_FIFO, O_WRONLY | O_NONBLOCK);
	if (fd >= 0) {
		status = fstat(fd, &st);
		if (status) {
			return error;
		}
		if (!S_ISFIFO(st.st_mode)) {
			close(fd);
			return invalid;
		}
		status = close(fd);
		if (status) {
			return error;
		}
		return running;
	} else {
		if (errno == ENXIO) {
			return stopped;
		} else if (errno == EISDIR || errno == EACCES
			|| errno == ENOENT) {
			return invalid;
		} else {
			return error;
		}
	}
}


bool serverAddJob(struct job job)
{
	(void) job;
	exit(1);
}

bool serverClose(bool killRunning)
{
	(void) killRunning;
	exit(1);
	//TODO:
	//assert that server is running
	//clear some shouldRun bool
	//wake server
	//sleep? wait? idk.
	//return !serverRunning
}

void serverMain(char *serverDir)
{
	(void) serverDir;
	exit(1);
	/*while (shouldRun) {
		wait(); // reader thread can wake us
		if (queue.peek() fits) {
			deque
			run
		}
	}*/
}
