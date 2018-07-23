/*
 * Src/server.c
 *
 * implements Src/server.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

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

enum serverState serverStatus(const char *serverDir)
{
	// TODO race conditions exist. Are they harmful? Fix before release.
	// using fstatat, openat, etc. should be sufficient.
	struct stat st;
	int ret;
	ret = stat(serverDir, &st);
	if (ret != 0) {
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

	ret = chdir(serverDir);
	if (ret != 0) {
		return error;
	}

	int fd = open(SFILE_FIFO, O_WRONLY | O_NONBLOCK);
	if (fd >= 0) {
		if (!S_ISFIFO(fd)) {
			close(fd);
			return invalid;
		}
		ret = close(fd);
		if (ret) {
			return error;
		}
		return running;
	} else {
		if (errno == ENXIO) {
			return stopped;
		} else if (errno == EISDIR || errno == EACCES) {
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
