/*
 * Src/server.c
 *
 * implements Src/server.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

//for O_DIRECTORY & dprintf
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "server.h"

//TODO make these names consistent with SFILE_FIFO? or maybe not, because these
//are not part of the public interface.
#define LOG "log.txt"
#define ERR "err.txt"

static struct server *this = NULL;

int serverAddJob(struct job job, bool isPriority)
{
	(void) job;
	(void) isPriority;
	fprintf(this->err, "Call to unimplemented function serverAddJob\n");
	fflush(this->err);
	return 1;
}

int serverShutdown(bool killRunning)
{
	(void) killRunning;
	fprintf(this->err, "Exiting abruptly, as graceful shutdowns are not yet implemented\n");
	exit(1);
	//TODO:
	//assert that server is running
	//clear some shouldRun bool
	//wake server
	//sleep? wait? idk.
	//return !serverRunning
}

__attribute__((noreturn)) void serverMain(void *srvr)
{
	this = srvr;
	while (1) {
		fprintf(this->log, "Server lives!\n");
		sleep(3);
	}
	exit(1);
	/*while (shouldRun) {
		wait(); // reader thread can wake us
		if (queue.peek() fits) {
			deque
			run
		}
	}*/
}

static int openServerDir(const char *path, int *fileDescriptor)
{
	int status;

	mkdir(path, SERVER_DIR_PERMS);
	//ignore mkdir errors, because it's possible to recover from them
	//if a valid server dir already exists.
	int fd = open(path, O_RDONLY | O_DIRECTORY);
	if (fd == -1) {
		return 1;
	}

	struct stat st;
	status = fstat(fd, &st);
	if (status) {
		return 1;
	}
	if (st.st_uid != geteuid()) {
		return 1;
	}

	*fileDescriptor = fd;
	return 0;
}

// opens the existing fifo name in the directory specified by fd. Only succeeds
// if there's something already reading from the fifo. On failure, fdFIFO is set
// to -1.
static enum serverInitCode openFIFO(int serverfd, const char *name, int *fdFIFO)
{
	int status;
	mkfifoat(serverfd, name, SERVER_DIR_PERMS);
	int fd = openat(serverfd, name, O_WRONLY | O_NONBLOCK);
	*fdFIFO = fd;
	if (fd >= 0) {
		struct stat st;
		status = fstat(fd, &st);
		if (status) {
			close(fd);
			return SIC_failed;
		}
		if (!S_ISFIFO(st.st_mode)) {
			close(fd);
			return SIC_failed;
		}
		return SIC_running;
	} else {
		if (errno == ENXIO) {
			return SIC_initialized;
		} else {
			return SIC_failed;
		}
	}
}

enum serverInitCode serverInitialize(const char *path, struct server *s)
{
	s->log = NULL;
	s->err = NULL;
	s->server = -1;
	s->fifo = -1;

	int status;

	status = openServerDir(path, &s->server);
	if (status) {
		goto fail;
	}

	int fd;
	fd = openat(s->server, LOG, O_WRONLY | O_CREAT, SERVER_DIR_PERMS);
	if (fd == -1) {
		goto fail;
	}
	s->log = fdopen(fd, "a");
	if (!s->log) {
		goto fail;
	}

	fd = openat(s->server, ERR, O_WRONLY | O_CREAT, SERVER_DIR_PERMS);
	if (fd == -1) {
		goto fail;
	}
	s->err = fdopen(fd, "a");
	if (!s->err) {
		goto fail;
	}

	//TODO close fd's if we can't open FIFO
	return openFIFO(s->server, SFILE_FIFO, &s->fifo);


fail:
	if (s->log) {
		fclose(s->log);
	}
	if (s->err) {
		fclose(s->err);
	}
	if (s->fifo != -1) {
		close(s->fifo);
	}
	if (s->server != -1) {
		close(s->server);
	}
	return SIC_failed;
}
