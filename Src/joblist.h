#ifndef JOBLIST_H
#define JOBLIST_H
/*
 * Src/joblist.h
 *
 * A thread-safe queue for storing `struct job`s.
 *
 * Copyright(C) 2018-2020, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>
#include <stdbool.h>

#include "job.h"
#include "server.h"

/*
 * Initialize the list; this function must be called before calling any other
 * function in this module.
 *
 * Returns zero when successful, and one if memory allocation fails.
 *
 * The behavior is undefined if the list is already initialized.
 */
int listInitialize();

/*
 * Add the given job to the queue. If isPriority is true, it is added to the
 * front of the queue, otherwise it is added to the back.
 */
void listAdd(struct job, bool isPriority);

/*
 * Returns the number of jobs that are "currently" in the list. Be careful when
 * using this list in multiple threads, as the value may be out of date even at
 * the instant it is returned.
 */
size_t listSize(void);

/*
 * Removes the head of the list and assigns it to `*out`.
 *
 * Returns zero when successful, and one if the list is empty.
 *
 * `out` may be null; the assignment is not performed in that case, and the
 * behavior of listNext is otherwise unchanged.
 */
int listNext(struct job *out);


/*
 * Copys the head of the list to `*out`.
 *
 * The behavior is undefined if `out` is null.
 *
 * Returns zero when successful.
 */
int listPeek(struct job *out);

/*
 * Destroy the list, freeing any memory that was allocated by this module.
 */
void listFree(void);

#ifdef TEST
size_t listCurCapacity(void);
#endif

#endif
