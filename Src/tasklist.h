#ifndef QUEUE_H
#define QUEUE_H
/*
 * Src/tasklist.h
 *
 * Maintains a single queue of jobs
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>
#include <stdbool.h>

#include "job.h"
#include "server.h"

void listAdd(struct job, bool isPriority);
size_t listSize(void);

/*
 * if listSize() > 0:
 *     Removes and returns the next item from the list.
 * else:
 *     return JOB_ZEROS
 */
struct job listNext(void);


struct job listPeek(void);
void listFree(void);

#ifdef TEST
size_t listCurCapacity(void);
#endif

#endif
