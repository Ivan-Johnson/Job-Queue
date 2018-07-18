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
#include "server.h"

void queueEnqueue(struct job);
size_t queueSize();
struct job queueDequeue();
struct job queuePeek();

#endif
