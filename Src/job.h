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
#include <sys/types.h>

struct job {
	// index 0 is the command we run. Indicies [1, argc] are arguments
	char *cmd;
};

//Given a *fully initialized job*, modifies buf such that it can be used by
//unserializeJob can be used to reconstruct the given job. Returns the number of
//chars used.
//
//Will only fail if the given job cannot fit in the specified buffer. In this
//case, returns -1.
ssize_t serializeJob(struct job job, char *buf, size_t bufLen)
	__attribute__((nonnull (2)));

//Always succeeds (when given valid arguments)
//
//No memory will be malloc'ed during this call. However, job may contain
//pointers to data in the buffer.
void unserializeJob(struct job *job, char *buf, size_t serialLen)
	__attribute__((nonnull (1, 2)));

#endif
