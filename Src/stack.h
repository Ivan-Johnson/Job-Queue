#ifndef STACK_H
#define STACK_H
/*
 * Src/stack.h
 *
 * Maintains a single stack of jobs
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "job.h"
#include "server.h"

void stackPush(struct job job);
size_t stackSize(void);

//Behavior is undefined if there is nothing on the stack
struct job stackPop(void);

//Behavior is undefined if there is nothing on the stack
struct job stackPeek(void);

void stackFree(void);

#ifdef TEST
size_t stackCurCapacity(void);
#endif

#endif
