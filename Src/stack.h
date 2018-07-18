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
#include "server.h"

void stackPush(struct job job);
size_t stackSize();

//Behavior is undefined if there is nothing on the stack
struct job stackPop();

//Behavior is undefined if there is nothing on the stack
struct job stackPeek();

#ifdef TEST
void stackPURGE();
#endif

#endif
