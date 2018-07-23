/*
 * Test/testServer.c
 *
 * Tests the server
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
//for mkfifoat:
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unity.h>

#include "server.h"

#define SDIR "/tmp/JormungandrServerTest/"

void setUp()
{
	//NOP
}

void tearDown()
{
	//NOP
}

/*
 * Runs "rm -rf" to recursively delete file
 *
 * file may not contain the character '\''
 *
 * strlen(file) must be less than PATH_MAX, which is assumed to be much less
 * than INT_MAX.
 */
static int rmrf(char *file)
{
#define FORMAT "rm -rf '%s'"
	size_t lenform = strlen(FORMAT) - 2; //-2 for %s
	size_t lenfile = strlen(file);
	char *cmd = malloc(sizeof(char) * (lenfile + lenform + 1));
	if (!cmd) {
		return -1;
	}
	int status = sprintf(cmd, FORMAT, file);
	if (status != (int) (lenfile + lenform)) {
		status = -1;
		goto freecmd;
	}
	status = system(cmd);
freecmd:
	free(cmd);
	return status;
#undef FORMAT
}

static int setupServerStatusTest(char *serverDir, bool serverDirExists,
			bool validPerms, bool mkFIFOfile,
			bool mkFIFOFIFO)
{
	int status = rmrf(serverDir);
	if (!serverDirExists || status) {
		return status;
	}

	mode_t perms = S_IRWXU | (validPerms ? 0 : S_IRWXG);
	status = mkdir(serverDir, perms);
	if (status == -1) {
		return 1;
	}

	int serverfd = open(serverDir, O_RDONLY);
	if (serverfd == -1) {
		return 1;
	}
	if (mkFIFOfile) {
		if (mkFIFOFIFO) {
			status = mkfifoat(serverfd, SFILE_FIFO, S_IRWXU);
			if (!status) {
				return status;
			}
		} else {
			int fd = openat(serverfd, SFILE_FIFO,
				O_WRONLY|O_CREAT, S_IRWXU | S_IRWXG);
			if (fd == -1) {
				return 1;
			}
			status = close(fd);
			if (status) {
				return status;
			}
		}
	}

	return 0;
}

static int openFIFORead(const char *serverDir, int *fd)
{
	int serverfd = open(serverDir, O_RDONLY);
	if (serverfd == -1) {
		return 1;
	}
	*fd = openat(serverfd, serverDir, O_RDONLY | O_NONBLOCK);
	if (*fd == -1) {
		close(serverfd);
		return 1;
	}
	int status = close(serverfd);
	if (status) {
		close(*fd);
		return 1;
	}
	return 0;
}

void runSStest(bool serverDirExists, bool validPerms, bool mkFIFOfile,
	bool mkFIFOFIFO, bool read, enum serverState expected)
{

	int status;
	status = setupServerStatusTest(SDIR, serverDirExists, validPerms,
				mkFIFOfile, mkFIFOFIFO);
	if (status) {
		TEST_IGNORE_MESSAGE("Failed to create the serverdir");
		return;
	}

	int fifofd;
	if (read) {
		status = openFIFORead(SDIR, &fifofd);
		if (status) {
			TEST_IGNORE_MESSAGE("Could not open FIFO");
			return;
		}
	}

	enum serverState ss = serverStatus(SDIR);

	if (mkFIFOfile && read) {
		status = close(fifofd);
		if (status) {
			TEST_IGNORE_MESSAGE("Could not close FIFO");
		}
	}

	TEST_ASSERT_EQUAL(expected, ss);
}



void testSSstopped()
{
	runSStest(true,  true,  true,  true,  false, stopped);
}

void testSSbadfifo()
{
	runSStest(true,  true,  true,  false, true,  invalid);
}

void testSSnofifo()
{
	runSStest(true,  true,  false, true,  false,  invalid);
}

void testSSbadperms()
{
	runSStest(true,  false, true,  true,  true,  invalid);
}

void testSSnoserver()
{
	runSStest(false, true,  true,  true,  false,  invalid);
}
