/*
 * Src/server.c
 *
 * implements Src/server.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

enum serverState serverStatus(char *serverDir)
{
	exit(1);
	(void) serverDir;
	//check that the dir exists with appropriate permissions
	//check that the pid specified in serverDir is running and matches:
	//        command name
	//        user
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
