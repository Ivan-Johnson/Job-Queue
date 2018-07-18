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
static struct job *jobs;
static size_t arr_len, next;

void stackPush(struct job job)
{
	(void) job;
	(void) arr_len;
	(void) next;
	(void) jobs;
	exit(1);
}

size_t stackSize()
{
	exit(1);
}

struct job stackPop()
{
	exit(1);
}

struct job stackPeek()
{
	exit(1);
}
