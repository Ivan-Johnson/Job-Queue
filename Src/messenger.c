/*
 * Src/messenger.c
 *
 * Implements Src/messenger.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "messenger.h"

bool messengerSendJob(char *serverDir, struct job job)
{
	(void) serverDir;
	(void) job;
	exit(1);
	//TODO:
	//return false if server not running
	//check status of fifo, initialize if dne
	//write job to fifo
}

bool messengerInitialize(char *serverDir)
{
	(void) serverDir;
	exit(1);
}
