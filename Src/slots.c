/*
 * Src/slots.c
 *
 * Implements Src/slots.h
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#include "slots.h"

struct slot {
	bool availible;
	pid_t pid;
};

// numSlots > 0 indicates that the slots module is initialized properly
unsigned int numSlots = 0;
struct slot *slots;
pid_t *scratch;

int slotsMalloc(unsigned int n)
{
	assert(n > 0);
	assert(numSlots == 0);
	exit(EXIT_FAILURE); // not yet implemented
}

void slotsFree()
{
	assert(numSlots > 0);
	exit(EXIT_FAILURE); // not yet implemented
}

unsigned int slotsAvailible()
{
	assert(numSlots > 0);
	// could cache result, but why bother optimizing prematurely?
	exit(EXIT_FAILURE); // not yet implemented
}

int slotsRegister(pid_t pid, unsigned int n, unsigned int *slots)
{
	assert(numSlots > 0);
	exit(EXIT_FAILURE); // not yet implemented
	(void) pid;
	for (unsigned int x = 0; x < n; x++) {
		// find slot
		if (slots != NULL) {
			//slots[x] = slot;
		}
	}
}

void slotsRelease(pid_t pid)
{
	assert(numSlots > 0);
	(void) pid;
	exit(EXIT_FAILURE); // not yet implemented
}
