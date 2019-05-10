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
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "job.h"
#include "tasklist.h"
#include "server.h"
#include "slots.h"
#include "messenger.h"

// TODO make these names consistent with SFILE_FIFO? or maybe not, because these
// are not part of the public interface.
//
// TODO why /aren't/ these part of the public interface? That way they could be
// incorperated into the argp help documentation.
#define LOG "log.txt"
#define ERR "err.txt"
#define FPORT "port.txt"

#define SLOT_ENVVAR "CUDA_VISIBLE_DEVICES"
#define MAX_ENVAL_LEN 10000

struct server {
	// fd of the main server directory
	int server;

	// fd of the fifo file used to receive requests (RD_ONLY)
	int fifo;

	// the port that this server uses to communicate with clients
	unsigned int port;

	// A file to use in place of the server's stdout
	FILE *log;

	// A file to use in place of the server's stderr
	FILE *err;

	unsigned int numSlots;

	// buffer for the EXCLUSIVE use of the server thread
	// guaranteed have a length of at least numSlots
	unsigned int *slotBuff;
};

static struct server concrete;
static struct server *this = NULL;

static struct server serverInitialize(void)
{
	struct server s;
	s.log = NULL;
	s.err = NULL;
	s.server = -1;
	s.port = 0;
	s.fifo = -1;
	s.numSlots = 0;
	s.slotBuff = NULL;
	return s;
}

void serverClose()
{
	if (this == NULL) {
		return;
	}

	if (this->log) {
		fclose(this->log);
	}
	if (this->err) {
		fclose(this->err);
	}
	if (this->fifo != -1) {
		close(this->fifo);
	}
	if (this->server != -1) {
		close(this->server);
	}
	if (this->slotBuff) {
		free(this->slotBuff);
	}

	this = NULL;
}

int serverAddJob(struct job job)
{
	listAdd(job, job.priority);
	//TODO wake server thread
	return 0;
}

int serverShutdown(bool killRunning)
{
	(void)killRunning;
	assert(this != NULL);
	fprintf(this->err, "Doing \"graceful\" shutdown (actually unsafe)\n");
	serverClose();
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
//
// The current thread MUST already have the mutex lock.
static struct job getJob()
{
	struct job job;

	job = listNext();
	if (listSize() > 0) {
		job = listNext();
	} else {
		job = JOB_ZEROS;
	}

	return job;
}

static int constructEnvval(size_t slotc, unsigned int *slotv, size_t buflen,
			   char *buf)
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
		size_t chars = (size_t)snprintf(buf + offset, space,
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
	assert(this);
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

	execv(job.argv[0], job.argv);	// no return unless it fails
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
				break;
			} else {
				assert(errno == EINTR);
				continue;
			}
		}

		if (pid == 0) {
			break;
		}

		if (WIFSIGNALED(status)) {
			slotsRelease(pid);
			// WTERMSIG(status)
		} else if (WIFEXITED(status)) {
			slotsRelease(pid);
			// WEXITSTATUS(status)
		}
	}
}

static void runJobs()
{
	int fail;

	while (1) {
		struct job job = getJob();
		if (jobEq(job, JOB_ZEROS)) {
			break;
		}

		if (slotsAvailible() < job.slots) {
			listAdd(job, true);
			break;
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
	}
}

__attribute__((noreturn))
void serverMain(void *srvr)
{
	this = srvr;
	assert(this->numSlots > 0);
	int fail = slotsMalloc(this->numSlots);
	if (fail) {
		fprintf(this->err, "Could not initialize slots module\n");
		exit(1);
	}

	fail = listInitialize();
	assert(!fail);

	while (1) {
		fflush(this->log);
		sleep(3);
		monitorChildren();
		fprintf(this->log, "tasks: %zd; free slots: %u\n",
			listSize(), slotsAvailible());

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

/*
 * Attempts to initialize s in preparation for launching a server.
 *
 * Returns 0 on success, nonzero on failure
 *
 * numSlots > 0
 */
int serverOpen(int dirFD, unsigned int numSlots, unsigned int port)
{
	assert(numSlots > 0);
	concrete = serverInitialize();
	this = &concrete;

	this->server = dirFD;
	this->numSlots = numSlots;
	this->port = port;

	int fd;

	fd = openat(dirFD, FPORT, O_WRONLY | O_CREAT | O_CLOEXEC,
		SERVER_DIR_PERMS);
	if (fd < 0) {
		return 1;
	}
	// TODO: do this intelligently (search for other instances of this
	// problem; C-s 627)
	unsigned int numChars = 627;
	char *buf = malloc(sizeof(char) * numChars);
	if (buf == NULL) {
		close(fd);
		return 1;
	}
	snprintf(buf, numChars, "%d\n", this->port);
	size_t strlen = strnlen(buf, numChars);
	assert(strlen < numChars);
	write(fd, buf, strlen);
	free(buf);
	// TODO ASAP: change file referenced by fd to be read only
	close(fd);

	// log file
	fd = openat(this->server, LOG, O_WRONLY | O_CREAT | O_CLOEXEC,
		    SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose();
		return 1;
	}
	// TODO: why is this (and ERR) being opened a second time? Can the
	// append mode not be done with the initial openat?
	this->log = fdopen(fd, "a");
	if (!this->log) {
		serverClose();
		return 1;
	}

	// err file
	fd = openat(this->server, ERR, O_WRONLY | O_CREAT | O_CLOEXEC,
		    SERVER_DIR_PERMS);
	if (fd < 0) {
		serverClose();
		return 1;
	}
	this->err = fdopen(fd, "a");
	if (!this->err) {
		serverClose();
		return 1;
	}

	// slotBuff
	this->slotBuff = malloc(sizeof(unsigned int) * this->numSlots);
	if (!this->slotBuff) {
		serverClose();
		return 1;
	}

	//TODO when *launching* the server, the reader creates its own fd. When
	//scheduling a command, we don't even call this function. So we don't
	//actually need a fifo fd in server, do we?
	mkfifoat(this->server, SFILE_FIFO, SERVER_DIR_PERMS);
	this->fifo = openat(this->server, SFILE_FIFO,
			 O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (this->fifo < 0) {
		serverClose();
		return 1;
	}
	return 0;
}

unsigned int serverGetPort(int serverdir)
{
	int fdPort = openat(serverdir, FPORT, O_RDONLY);

	// TODO: do this intelligently (search for other instances of this
	// problem; C-s 627)
	unsigned int numChars = 627;

	char *buf = malloc(sizeof(char) * numChars);
	if (buf == NULL) {
		return 0;
	}

	ssize_t s = read(fdPort, buf, numChars);
	if (s <= 0 || s == numChars) {
		free(buf);
		return 0;
	}

	long l = atol(buf);
	if (l <= 0 || l > UINT_MAX) {
		free(buf);
		return 0;
	}

	free(buf);
	return (unsigned int) l;
}

int serverForkNew(int fd, unsigned int numSlots, unsigned int port)
{
	if (numSlots == 0) {
		numSlots = 1;
	}
	int status;
	status = serverOpen(fd, numSlots, port);
	if (status) {
		return 1;
	}

	int pid = fork();
	if (pid == -1) {
		puts("Failed to fork a server");
		serverClose();
		return 1;
	} else if (pid != 0) {
		puts("Successfully forked a server");
		serverClose();
		return 0;
	}

	status = setsid();
	if (status == -1) {
		fprintf(this->err, "Failed to setsid: %s\n", strerror(errno));
		exit(1);
	}

	struct messengerReaderArgs args;
	args.log = this->log;
	args.err = this->err;
	args.server = this->server;
	pthread_t unused;
	status =
	    pthread_create(&unused, NULL, messengerReader, (void *)&args);
	if (status) {
		fprintf(this->err, "Failed to start reader thread: %s\n",
			strerror(status));
		exit(1);
	}

	serverMain((void *)&concrete);
}
