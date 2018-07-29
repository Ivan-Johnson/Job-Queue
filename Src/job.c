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

ssize_t serializeJob(struct job job, char *buf, size_t bufLen)
{
	size_t space = bufLen;

	size_t len = strlen(job.cmd) + 1;
	if (len > space) {
		return -1;
	}
	space -= len;
	memcpy(buf, job.cmd, len);

	size_t used = bufLen - space;
	assert(used <= SSIZE_MAX);
	return (ssize_t) used;
}

void unserializeJob(struct job *job, char *buf, size_t serialLen)
{
	size_t len = strnlen(buf, serialLen);
	assert(len != serialLen);
	job->cmd = buf;
}
