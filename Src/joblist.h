#ifndef QUEUE_H
#define QUEUE_H
/*
 * Src/joblist.h
 *
 * Maintains a single queue of jobs
 *
 * Copyright(C) 2018-2019, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>
#include <stdbool.h>

#include "job.h"
#include "server.h"

int listInitialize();

void listAdd(struct job, bool isPriority);

/*
 * Returns the number of jobs that are "currently" in the list. Be careful when
 * using this list in multiple threads, as the value may be out of date even at
 * the instant it is returned.
 */
size_t listSize(void);

/*
 * if listSize() > 0:
 *     Removes and assigns it to (*out).
 *     return 0
 * else:
 *     return 1
 *
 * If (out == NULL), this function simply removes the first element if it
 * exists, and returns the appropriate value.
 *
 * NOTE: be wary of race conditions when checking listSize. In general, it is
 * preferred to simply call listNext and check the return value rather than
 * check with listSize before calling listNext.
 */
int listNext(struct job *out);


/*
 * Identical to listNext, except that:
 *
 * A: listPeek never removes the job from the list.
 *
 * B: out cannot be NULL
 */
int listPeek(struct job *out);
void listFree(void);

#ifdef TEST
size_t listCurCapacity(void);
#endif

#endif
