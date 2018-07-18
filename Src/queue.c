/*
 * Src/queue.c
 *
 * implements Src/queue.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdlib.h>

#include "server.h"

//we store the array in a circular array.

//old is the item that will come off next. New is the index after the item that
//was most recently put on. Valid indicies are old, index(old+1), .. index(new-1)

//if new==old, the queue is empty

//if index(new+1) == old, then the array is full.

//NOTE that we're using the unsigned type "size_t" for indicies. As a
//consequence, we're not going to store anything at index zero, and have
//index(0) be the last index in the array.

//NOTE: two indicies of the array are wasted. Index zero and the cell at new.
//So the queue can store arr_len - 2 values without needing to be enlarged.

static struct job *jobs;
static size_t arr_len, old, new;

/*
 * Map values in the range [0, arr_len] back to the valid index range
 * [1,arr_len). i.e. given a valid index i, while(1) {i = index(i + delta); }
 * loops over valid indicies if delta is ±1.
 *
 * NOTE: index(i - 2) != index(index(i - 1) - 1) when i == (size_t) 1; so,
 * indicies cannot be modified by more than one at a time.
 */
static size_t index(size_t pseudoindex)
{
	//TODO write tests, THEN optimize away the if statements
	if (pseudoindex == 0) {
		return arr_len - 1;
	} else if (pseudoindex == arr_len) {
		return 1;
	} else {
		return pseudoindex;
	}
}

void queueEnqueue(struct job job)
{
	(void) job;
	(void) index;
        (void) jobs;
	(void) arr_len;
	(void) old;
	(void) new;
	exit(1);
}

size_t queueSize()
{
	exit(1);
}

struct job queueDequeue()
{
	exit(1);
}

struct job queuePeek()
{
	exit(1);
}
