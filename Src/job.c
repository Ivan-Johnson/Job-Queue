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

	int argc = 0;
	while (job.argv[argc] != NULL) {
		argc++;
	}

	len = sizeof(int);
	if (len > space) {
		return -1;
	}
	space -= len;
	memcpy(buf, &argc, len);
	buf += len;

	for (int x = 0; x < argc; x++) {
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
		size_t capfree, char **bufEnd)
{
	size_t sizetmp;
	int argc;

	sizetmp = sizeof(unsigned int) + sizeof(bool) + sizeof(int);
	assert(sizetmp <= capfree);
	capfree -= sizetmp;
	memcpy(&job->slots, buf, sizeof(unsigned int));
	buf += sizeof(unsigned int);
	memcpy(&job->priority, buf, sizeof(bool));
	buf += sizeof(bool);
	memcpy(&argc, buf, sizeof(int));
	buf += sizeof(int);

	job->argv = malloc(sizeof(char*) * ((unsigned long) argc + 1));
	if (!job->argv) {
		return 1;
	}

	for (int x = 0; x < argc; x++) {
		sizetmp = strnlen(buf, capfree);
		if (sizetmp == capfree) {
			free(job->argv);
			return 1;
		}
		capfree -= sizetmp;
		job->argv[x] = buf;
		buf += sizetmp + 1;
	}

	if (bufEnd != NULL) {
		*bufEnd = buf;
	}

	return 0;
}

void freeUnserializedJob(struct job job)
{
	free(job.argv);
}

// Returns true if the jobs are equivalent, otherwise return false
bool jobEq(struct job job1, struct job job2)
{
	if (job1.slots != job2.slots) {
		return false;
	}
	if (job1.priority != job2.priority) {
		return false;
	}

	// compare argv's
	if (job1.argv == job2.argv) {
		return true;
	} else if (job1.argv == NULL || job2.argv == NULL) {
		return false;
	}
	int i = 0;
	while (job1.argv[i] != NULL && job2.argv[i] != NULL) {
		if (job1.argv[i] == job2.argv[i]) {
			continue;
		}
		if (strcmp(job1.argv[i], job2.argv[i])) {
			return false;
		}
		i++;
	}
	return job1.argv[i] == job2.argv[i];
}

int cloneJob(struct job *dest, struct job src)
{
	dest->slots = src.slots;
	dest->priority = src.priority;
	int argc = 0;
	while (src.argv[argc] != NULL) {
		argc++;
	}
	dest->argv = malloc(sizeof(char*) * ((size_t) argc + 1));
	if (!dest->argv) {
		return 1;
	}
	int x;
	for(x = 0; x < argc; x++) {
		size_t len = strlen(src.argv[x]) + 1;
		dest->argv[x] = malloc(sizeof(char) * len);
		if (!dest->argv[x]) {
			goto fail;
		}
		memcpy(dest->argv[x], src.argv[x], len);
	}
	dest->argv[argc] = NULL;
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
	assert(job.argv);
	int x = 0;
	while (job.argv[x] != NULL) {
		free(job.argv[x]);
		x++;
	}
	free(job.argv);
}
