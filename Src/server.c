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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "job.h"
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
	fprintf(this->err, "Call to unimplemented function %s\n", __func__);
	fflush(this->err);
	return 1;
}

int serverShutdown(bool killRunning)
{
	(void) killRunning;
	assert(this != NULL);
	fprintf(this->err, "Exiting abruptly, as graceful shutdowns are not yet implemented\n");
	serverClose(*this);
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
	/*while (shouldRun) {
		wait(); // reader thread can wake us
		if (queue.peek() fits) {
			deque
			run
		}
	}*/
}

int getServerDir(const char *path)
{
	int status;

	mkdir(path, SERVER_DIR_PERMS);
	//ignore mkdir errors, because it's possible to securely recover from
	//them if a valid server dir already exists.
	int fd = open(path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) {
		return -1;
	}

	struct stat st;
	status = fstat(fd, &st);
	if (status) {
		return -1;
	}
	if (st.st_uid != geteuid()) {
		return -1;
	}
	if ((st.st_mode & 0777) != SERVER_DIR_PERMS) {
		return -1;
	}

	return fd;
}

static struct server serverInitialize(void)
{
	struct server s;
	s.log = NULL;
	s.err = NULL;
	s.server = -1;
	s.fifo = -1;
	return s;
}

void serverClose(struct server s)
{
	if (s.log) {
		fclose(s.log);
	}
	if (s.err) {
		fclose(s.err);
	}
	if (s.fifo != -1) {
		close(s.fifo);
	}
	if (s.server != -1) {
		close(s.server);
	}
}

int openServer(int dirFD, struct server *s)
{
	*s = serverInitialize();
	s->server = dirFD;

	int fd;
	fd = openat(s->server, LOG, O_WRONLY | O_CREAT, SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose(*s);
		return 1;
	}
	s->log = fdopen(fd, "a");
	if (!s->log) {
		serverClose(*s);
		return 1;
	}
	fd = openat(s->server, ERR, O_WRONLY | O_CREAT, SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose(*s);
		return 1;
	}
	s->err = fdopen(fd, "a");
	if (!s->err) {
		serverClose(*s);
		return 1;
	}

	mkfifoat(s->server, SFILE_FIFO, SERVER_DIR_PERMS);
	s->fifo = openat(s->server, SFILE_FIFO, O_RDONLY | O_NONBLOCK);
	if (s->fifo < 0) {
		serverClose(*s);
		return 1;
	}
	return 0;
}
