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
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "job.h"
#include "queue.h"
#include "server.h"
#include "stack.h"
#include "slots.h"

//TODO make these names consistent with SFILE_FIFO? or maybe not, because these
//are not part of the public interface.
#define LOG "log.txt"
#define ERR "err.txt"

#define SLOT_ENVVAR "CUDA_VISIBLE_DEVICES"
#define MAX_ENVAL_LEN 10000

static struct server *this = NULL;
static pthread_mutex_t lock;

int serverAddJob(struct job job)
{
	pthread_mutex_lock(&lock);
	if (job.priority) {
		stackPush(job);
	} else {
		queueEnqueue(job);
	}
	pthread_mutex_unlock(&lock);
	//TODO wake server thread
	return 0;
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

// If there are no jobs to run, then this function returns JOB_ZEROS, otherwise
// it returns the job that should be run next.
//
// This function shall never fail.
static struct job getJob()
{
	struct job job;
	int fail;

	fail = pthread_mutex_lock(&lock);
	assert(!fail);
	if (stackSize() > 0) {
		job = stackPop();
	} else if (queueSize() > 0) {
		job = queueDequeue();
	} else {
		job = JOB_ZEROS;
	}
	fail = pthread_mutex_unlock(&lock);
	assert(!fail);

	return job;
}

static int constructEnvval(size_t slotc, unsigned int *slotv, size_t buflen, char *buf)
{
	assert(slotc > 0);
	size_t offset = 0;

	for (size_t s = 0; s < slotc; s++) {
		size_t space = buflen - offset;
		char *fstring;
		if (s == slotc - 1) {
			fstring = "%u";
		} else {
			fstring = "%u,";
		}
		size_t chars = (size_t) snprintf(buf + offset, space,
				fstring, slotv[s]);
		if (chars == space) {
			return 1;
		}
		offset += chars;
	}
	return 0;
}

static int runJob(struct job job)
{
	unsigned int numslot = job.slots;
	assert(slotsAvailible() >= numslot);

	int fail = slotsReserveSet(numslot, this->slotBuff);
	if (fail) {
		return 1;
	}

	char envval[MAX_ENVAL_LEN];
	fail = constructEnvval(numslot, this->slotBuff, MAX_ENVAL_LEN, envval);
	if (fail) {
		slotsUnreserveSet(numslot, this->slotBuff);
		return 1;
	}
	fail = setenv(SLOT_ENVVAR, envval, true);
	if (fail) {
		slotsUnreserveSet(numslot, this->slotBuff);
		return 1;
	}

	int pid = fork();
	if (pid == -1) {
		slotsUnreserveSet(numslot, this->slotBuff);
		return 1;
	} else if (pid != 0) {
		slotsRegisterSet(pid, numslot, this->slotBuff);
		return 0;
	}

	execv(job.argv[0], job.argv); // no return unless it fails
	fprintf(this->err,
		"execv failed for \"%s\" command with \"%s\"\n",
		job.argv[0], strerror(errno));
	fflush(this->err);
	exit(1);
}

/*
 * Updates running
 */
static void monitorChildren()
{
	while (1) {
		int status;
		pid_t pid = waitpid(-1, &status, WNOHANG);
		if (pid == -1) {
			if (errno == ECHILD) {
				// all children have been handled
				return;
			} else {
				assert(errno == EINTR);
				continue;
			}
		}

		if (pid == 0) { // no child has a status update
			return;
		}

		if (WIFSIGNALED(status)) {
			slotsRelease(pid);
			// WTERMSIG(status)
		} else if(WIFEXITED(status)) {
			slotsRelease(pid);
			// WEXITSTATUS(status)
		}
	}
}

static void runJobs()
{
	static struct job job;
	int fail;

	while (1) {
		if (jobEq(job, JOB_ZEROS)) {
			job = getJob();
			if (jobEq(job, JOB_ZEROS)) {
				return;
			}
		}

		assert(!jobEq(job, JOB_ZEROS));

		if (slotsAvailible() < job.slots) {
			return;
		}

		fail = runJob(job);
		if (fail) {
			fprintf(this->err, "Failed to execute job \"%s\"\n",
				job.argv[0]);
			fflush(this->err);
		} else {
			fprintf(this->log, "Began executing \"%s\"\n",
				job.argv[0]);
		}
		freeJobClone(job);
		job = JOB_ZEROS;
	}
}

__attribute__((noreturn)) void serverMain(void *srvr)
{
	this = srvr;
	assert(this->numSlots > 0);
	if (pthread_mutex_init(&lock, NULL) != 0) {
		fprintf(this->err, "Could not initialize mutex\n");
		exit(1);
	}
	int fail = slotsMalloc(this->numSlots);
	if (fail) {
		fprintf(this->err, "Could not initialize slots module\n");
		exit(1);
	}

	while (1) {
		fflush(this->log);
		sleep(3);
		monitorChildren();
		fprintf(this->log, "Queue: %zd; Stack: %zd; free slots: %u\n",
			queueSize(), stackSize(), slotsAvailible());

		runJobs();
	}
}

int getServerDir(const char *path)
{
	int status;

	mkdir(path, SERVER_DIR_PERMS);
	//ignore mkdir errors, because it's possible to securely recover from
	//them if a valid server dir already exists.
	int fd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
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
	s.numSlots = 0;
	s.slotBuff = NULL;
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
	if (s.slotBuff) {
		free(s.slotBuff);
	}
}

int openServer(int dirFD, struct server *s, unsigned int numSlots)
{
	assert(numSlots > 0);
	*s = serverInitialize();
	s->server = dirFD;
	s->numSlots = numSlots;

	int fd;
	fd = openat(s->server, LOG, O_WRONLY | O_CREAT | O_CLOEXEC,
		SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose(*s);
		return 1;
	}
	s->log = fdopen(fd, "a");
	if (!s->log) {
		serverClose(*s);
		return 1;
	}
	fd = openat(s->server, ERR, O_WRONLY | O_CREAT | O_CLOEXEC,
		SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose(*s);
		return 1;
	}
	s->err = fdopen(fd, "a");
	if (!s->err) {
		serverClose(*s);
		return 1;
	}
	s->slotBuff = malloc(sizeof(unsigned int) * s->numSlots);
	if (!s->slotBuff) {
		serverClose(*s);
		return 1;
	}

	//TODO when *launching* the server, we reader creates its own fd. When
	//scheduling a command, we don't even call this function. So we don't
	//actually need a fifo fd in server, do we?
	mkfifoat(s->server, SFILE_FIFO, SERVER_DIR_PERMS);
	s->fifo = openat(s->server, SFILE_FIFO,
			O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (s->fifo < 0) {
		serverClose(*s);
		return 1;
	}
	return 0;
}
