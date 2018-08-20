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
unsigned int num = 0;
struct slot *slots;

int slotsMalloc(unsigned int c)
{
	assert(c > 0);
	assert(num == 0);
	num = c;

	slots = malloc(sizeof(struct slot) * num);
	if (!slots) {
		num = 0;
		return 1;
	}

	for (unsigned int a = 0; a < num; a++) {
		slots[a].availible = true;
		slots[a].pid = -1;
	}

	return 0;
}

void slotsFree()
{
	assert(num > 0);
	free(slots);
	num = 0;
}

unsigned int slotsAvailible()
{
	assert(num > 0);
	// do not optimize; Ο(num) is sufficiently fast
	unsigned int count = 0;
	for (unsigned int x = 0; x < num; x++) {
		if (slots[x].availible) {
			count++;
		}
	}
	return count;
}

int slotsReserveSet(unsigned int count, unsigned int *slotv)
{
	assert(num > 0);
	if (slotsAvailible() < count) {
		return 1;
	}
	static unsigned int slot = 0;
	unsigned int slot_init = slot;
	for (unsigned int x = 0; x < count; x++) {
		while (!slots[slot].availible) {
			slot = (slot + 1) % num;
			assert(slot != slot_init);
		}

		slots[slot].availible = false;
		slots[slot].pid = -1;

		slotv[x] = slot;
	}
	return 0;
}

void slotsUnreserveSet(unsigned int count, unsigned int *slotv)
{
	for (unsigned int x = 0; x < count; x++) {
		unsigned int slot = slotv[x];
		slots[slot].availible = true;
	}
}

void slotsRegisterSet(pid_t pid, unsigned int count, unsigned int *slotv)
{
	for (unsigned int x = 0; x < count; x++) {
		unsigned int slot = slotv[x];
		slots[slot].pid = pid;
	}
}

void slotsRelease(pid_t pid)
{
	assert(num > 0);
	// do not optimize; Ο(num) is sufficiently fast
	for (unsigned int x = 0; x < num; x++) {
		if (slots[x].pid == pid) {
			slots[x].availible = true;
		}
	}
}
