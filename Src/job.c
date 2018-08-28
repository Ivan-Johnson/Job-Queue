/*
 * Src/job.c
 *
 * implements Src/job.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

//for strnlen
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"

//serialized format is as follows:
//sizeof(uint) bytes: unsigned int slots
//sizeof(bool) bytes: priority bool
//sizeof(int) bytes: argc int
//sizeof(char) * (strlen(argv[x]) + 1) bytes: (repeated argc times)
//		the argv[x] string, including the terminating NULL byte

ssize_t serializeJob(struct job job, char *buf, size_t bufLen)
{
	size_t space = bufLen;
	size_t len;

	len = sizeof(unsigned int);
	if (len > space) {
		return -1;
	}
	space -= len;
	memcpy(buf, &job.slots, len);
	buf += len;

	len = sizeof(bool);
	if (len > space) {
		return -1;
	}
	space -= len;
	memcpy(buf, &job.priority, len);
	buf += len;

	len = sizeof(int);
	if (len > space) {
		return -1;
	}
	space -= len;
	memcpy(buf, &job.argc, len);
	buf += len;

	for (int x = 0; x < job.argc; x++) {
		len = strlen(job.argv[x]) + 1;
		if (len > space) {
			return -1;
		}
		space -= len;
		memcpy(buf, job.argv[x], len);
		buf += len;
	}

	size_t used = bufLen - space;
	assert(used <= SSIZE_MAX);
	return (ssize_t) used;
}

int unserializeJob(struct job *restrict job, char *restrict buf,
		size_t serialLen)
{
	size_t remLen = serialLen; //remaining length
	size_t len;

	len = sizeof(unsigned int) + sizeof(bool) + sizeof(int);
	assert(len <= remLen);
	remLen -= len;
	memcpy(&job->slots, buf, sizeof(unsigned int));
	buf += sizeof(unsigned int);
	memcpy(&job->priority, buf, sizeof(bool));
	buf += sizeof(bool);
	memcpy(&job->argc, buf, sizeof(int));
	buf += sizeof(int);

	job->argv = malloc(sizeof(char*) * ((unsigned long) job->argc));
	if (!job->argv) {
		return 1;
	}

	for (int x = 0; x < job->argc; x++) {
		len = strnlen(buf, remLen);
		assert(len < remLen);
		remLen -= len;
		job->argv[x] = buf;
		buf += len + 1;
	}

	return 0;
}

void freeUnserializedJob(struct job job)
{
	free(job.argv);
}

// Returns 0 if the jobs are equivalent, nonzero otherwise
bool jobEq(struct job job1, struct job job2)
{
	if (job1.slots != job2.slots) {
		return false;
	}
	if (job1.priority != job2.priority) {
		return false;
	}
	if (job1.argc != job2.argc) {
		return false;
	}
	if (job1.argv != job2.argv) {
		if (job1.argv == NULL || job2.argv == NULL) {
			return false;
		}
		for (int x = 0; x < job1.argc; x++) {
			if (strcmp(job1.argv[x], job2.argv[x])) {
				return false;
			}
		}
	}
	return true;
}

int cloneJob(struct job *dest, struct job src)
{
	dest->slots = src.slots;
	dest->priority = src.priority;
	dest->argc = src.argc;
	assert(src.argc >= 0);
	dest->argv = malloc(sizeof(char*) * ((size_t)src.argc + 1));
	if (!dest->argv) {
		return 1;
	}
	int x;
	for(x = 0; x < src.argc; x++) {
		size_t len = strlen(src.argv[x]) + 1;
		dest->argv[x] = malloc(sizeof(char) * len);
		if (!dest->argv[x]) {
			goto fail;
		}
		memcpy(dest->argv[x], src.argv[x], len);
	}
	dest->argv[src.argc] = NULL;
	return 0;
fail:
	for(x--; x >= 0; x--) {
		free(dest->argv[x]);
	}
	free(dest->argv);
	return 1;
}

void freeJobClone(struct job job)
{
	assert(job.argv && job.argc >= 0);
	for(int x = 0; x < job.argc; x++) {
		free(job.argv[x]);
	}
	free(job.argv);
}
