/*
 * Src/stack.c
 *
 * implements Src/stack.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "stack.h"

/* The stack is stored as an array
 *
 * When next == 0, the stack is empty. If next == arr_len, then the array is
 * full.
 *
 * Index 0 is the oldest item, index (next-1) is the most recent.
 */
static struct job *jobs = NULL;
static size_t arr_len = 0, next = 0;

void stackPURGE()
{
	if (jobs != NULL) {
		free(jobs);
		jobs = NULL;
	}
	arr_len = 0;
	next = 0;
}

void stackPush(struct job job)
{
	(void) job;
	(void) arr_len;
	(void) next;
	(void) jobs;
	next++;
}

size_t stackSize()
{
	return next;
}

struct job stackPop()
{
	next--;
	struct job j;
	j.cmd = NULL;
	return j;
}

struct job stackPeek()
{
	struct job j;
	j.cmd = NULL;
	return j;
}
