/*
 * Src/tasklist.c
 *
 * implements Src/tasklist.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
// for pthread_mutexattr_settype
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include "job.h"
#include "server.h"

//we store the array in a circular array.

//old is the item that will come off next. New is the index after the item that
//was most recently put on. The indicies that currently hold data are:
//old, index(old+1), … index(new-1)

//if new==old, the list is empty

//if index(new+1) == old, then the array is full.

//NOTE that we're using the unsigned type "size_t" for indicies. As a
//consequence, we're not going to store anything at index zero, and have
//index(0) be the last index in the array.

//NOTE: two indicies of the array are wasted. Index zero and the cell at new.
//So the list can store arr_len - 2 values without needing to be enlarged.

#define LIST_MIN_SIZE 128
static struct job *jobs = NULL;
static size_t arr_len = 0, old = 0, new = 0;
static pthread_mutex_t listMutex;

void listInitialize(void)
{
	assert(jobs == NULL);

	arr_len = LIST_MIN_SIZE;
	jobs = malloc(sizeof(struct job) * arr_len);
	old = 1;
	new = 1;

	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr)) {
		fprintf(stderr, "Could not initialize mutexattr\n");
		exit(1);
	}
	if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) {
		fprintf(stderr, "Could not set mutexattr to errorcheck\n");
		exit(1);
	}
	if (pthread_mutex_init(&listMutex, &attr)) {
		fprintf(stderr, "Could not initialize mutex\n");
		exit(1);
	}
}

size_t listCurCapacity(void)
{
	assert(jobs != NULL);
	return arr_len - 2;
}

void listFree(void)
{
	assert(jobs != NULL);
	assert(!pthread_mutex_lock(&listMutex));
	if (jobs != NULL) {
		free(jobs);
		jobs = NULL;
	}
	arr_len = 0;
	old = 1;
	new = 1;
	pthread_mutex_destroy(&listMutex);
}

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

static void listGrow(void)
{
	struct job *arrOld = jobs;
	jobs = malloc(sizeof(struct job) * arr_len * 2);

	size_t newNew = 1;
	for (size_t x = old; x != new; x = index(x + 1), newNew++) {
		jobs[newNew] = arrOld[x];
	}
	new = newNew;
	old = 1;
	// We can't update arr_len until after the loop, because index uses it
	arr_len *= 2;

	free(arrOld);
}

void listAdd(struct job job, bool isPriority)
{
	assert(jobs != NULL);
	assert(!pthread_mutex_lock(&listMutex));

	if (old == index(new + 1)) {	//list is full
		listGrow();
	}
	if (isPriority) {
		old = index(old-1);
		jobs[old] = job;
	} else {
		jobs[new] = job;
		new = index(new + 1);
	}

	assert(!pthread_mutex_unlock(&listMutex));
}

static size_t computeSize()
{
	//subtract two because index zero and index new are empty
	if (old <= new) {	// The list is not wrapped
		return new - old;
	} else {		//end of list loops back to start of the array
		// the total number of cells in the array, if we ignore index 0
		size_t num_cells = arr_len - 1;

		// the number of empty cells in the array:
		// [new, old)
		size_t num_empty = old - new;

		return num_cells - num_empty;
	}
}

size_t listSize(void)
{
	assert(jobs != NULL);
	assert(!pthread_mutex_lock(&listMutex));

	size_t ans = computeSize();

	assert(!pthread_mutex_unlock(&listMutex));
	return ans;
}

static inline void listShrink(void)
{
	size_t newSize = arr_len / 2;
	if (newSize < LIST_MIN_SIZE) {
		return;
	}

	struct job *arrOld = jobs;
	jobs = malloc(sizeof(struct job) * newSize);

	size_t newNew = 1;
	for (size_t x = old; x != new; x = index(x + 1), newNew++) {
		jobs[newNew] = arrOld[x];
	}
	new = newNew;
	old = 1;
	// We can't update arr_len until after the loop, because index uses it
	arr_len = newSize;

	assert(new < arr_len);

	free(arrOld);
}

struct job listNext(void)
{
	assert(jobs != NULL);
	assert(!pthread_mutex_lock(&listMutex));

	struct job job;
	if (old == new) {
		job = JOB_ZEROS;
	} else {
		job = jobs[old];
		old = index(old + 1);
		if (computeSize() < arr_len / 4) {
			listShrink();
		}
	}
	assert(!pthread_mutex_unlock(&listMutex));
	return job;
}

struct job listPeek(void)
{
	assert(jobs != NULL);
	assert(!pthread_mutex_lock(&listMutex));
	struct job ans = jobs[index(old)];
	assert(!pthread_mutex_unlock(&listMutex));
	return ans;
}
