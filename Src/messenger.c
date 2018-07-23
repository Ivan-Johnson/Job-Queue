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

bool messengerSendJob(int serverfd, struct job job)
{
	(void) serverfd;
	(void) job;
	exit(1);
	//TODO:
	//return false if server not running
	//check status of fifo, initialize if dne
	//write job to fifo
}

bool messengerInitialize(int serverfd)
{
	(void) serverfd;
	exit(1);
}
