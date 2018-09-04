#ifndef JORMUNGANDR_H
#define JORMUNGANDR_H
/*
 * Src/jormungandr.h
 *
 * Parses and fulfils arguments.
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <stdbool.h>

enum task {
	task_undefined,
	task_launch,
	task_schedule
};

struct arguments {
	enum task task;
	char *server;
	// cmd is NULL terminated.
	char **cmd;
	unsigned int slotsUse;
	unsigned int slotsMax;
	bool priority;
};

struct arguments parseArgs(int argc, char **argv);
int fulfilArgs(struct arguments args);

#endif
