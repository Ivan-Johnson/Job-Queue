#ifndef SLOTS_H
#define SLOTS_H
/*
 * Src/slots.h
 *
 * Tracks which processes are using which slots
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

/*
 * This module is used when you have a numerated set of equivalent resources
 * (e.g. GPUs) and you want to reserve members of that set for use by particular
 * processes. Members of the set of resources are called "slots". This module
 * simply tracks what slot belongs to which process.
 *
 * slots is NOT thread-safe.
 *
 * slotsMalloc MUST be called before using ANY other function in the slots
 * module. This allocated memory on the heap; after the last call to slots,
 * slotsFree can be called to free the memory allocated by slotsMalloc.
 */

#include <sys/types.h>

// Initializes the slots module for use with the specified number of slots.
//
// numSlots must be greater than zero.
//
// On success return 0; otherwise returns non-zero.
int slotsMalloc(unsigned int numSlots);
void slotsFree();

// Returns the number of slots that are currently availible.
// The slotsMalloc function must have already been called.
unsigned int slotsAvailible();

// Registers slotc slots for the process with pid pid.
//
// The given pid cannot currently be registered.
//
// Returns 0 if the slots have been successfully alocated, returns non-zero
// otherwise. (In particular, note that slotsRegister will fail if there are
// fewer than slotc availible slots)
//
// If slotv is non-null, then slotv[0] through slotv[slotc-1] will be set to the
// indicies of the slots that are to be used by pid. These values will be in the
// range [0, slotc). They are also guaranteed to only be used once accross all
// registered processes.
int slotsRegister(pid_t pid, unsigned int slotc, unsigned int *slotv);

// "Unregisters" the given pid, and frees its slot for reuse
//
// The given pid must currently be registered.
void slotsRelease(pid_t pid);

#endif
