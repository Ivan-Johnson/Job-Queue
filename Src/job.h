#ifndef JOB_H
#define JOB_H
/*
 * Src/job.h
 *
 * The job struct stores the information need to run a particular command
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <stdbool.h>
#include <sys/types.h>

struct job {
	bool priority;
	int argc;
	// index 0 is the command we run. Indicies [1, argc) are arguments
	char **argv;
};

//Given a *fully initialized job*, modifies buf such that it can be used by
//unserializeJob can be used to reconstruct the given job. Returns the number of
//chars used.
//
//Will only fail if the given job cannot fit in the specified buffer. In this
//case, returns -1.
ssize_t serializeJob(struct job job, char *buf, size_t bufLen)
	__attribute__((nonnull(2)));

//Memory will be malloc'ed to store argv. To free this memory, call
//freeUnserializedJob.
//
//Note that the char*'s in argv will be pointers to data in buf. As such, Buf
//should not be modified until the values in job are unneeded, and
//freeUnserializedJob has been called.
//
//job and buf cannot overlap eachother or serialLen
//
//Returns 0 on success, nonzero on fail
int unserializeJob(struct job *restrict job, char *restrict buf,
		size_t serialLen) __attribute__((nonnull(1, 2)));

void freeUnserializedJob(struct job);

// Returns true if the jobs are equivalent, false otherwise
bool jobEq(struct job job1, struct job job2);

int cloneJob(struct job *dest, struct job src);

void freeJobClone(struct job job);

#endif
