#ifndef QUEUE_H
#define QUEUE_H
/*
 * Src/queue.h
 *
 * Maintains a single queue of jobs
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "job.h"
#include "server.h"

void queueEnqueue(struct job);
size_t queueSize(void);
struct job queueDequeue(void);
struct job queuePeek(void);
void queueFree(void);

#ifdef TEST
size_t queueCurCapacity(void);
#endif

#endif
