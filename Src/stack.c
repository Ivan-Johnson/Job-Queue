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

#define STACK_INIT_ARR_LEN 128

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

//Initialize the stack if necessary
static inline void stackInitialize()
{
	if (jobs == NULL) {
		arr_len = STACK_INIT_ARR_LEN;
		jobs = malloc(sizeof(struct job) * arr_len);
		next = 0;
	}
}

void stackPush(struct job job)
{
	stackInitialize();
	jobs[next] = job;
	next++;
}

size_t stackSize()
{
	//Even pre-initialization the value of next is accurate
	//stackInitialize();
	return next;
}

struct job stackPop()
{
	next--;
	return jobs[next];
}

struct job stackPeek()
{
	return jobs[next - 1];
}
