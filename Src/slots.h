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

// Reserves a set of count slots for later use.
//
// slotv must point to a buffer of sufficient size to store count unsigned ints.
//
// Returns 0 if the set has successfully been reserved; returns non-zero
// otherwise.
//
// On success, slotv[0] through slotv[count-1] will be set to the indicies of
// the slots that are part of this set. These values will be in the range [0,
// slotc). Each value is guaranteed to not match any value that has been
// reserved for any pid.
//
// On success, the next function call to slots MUST be slotsRegisterSet or
// slotsUnreserveSet, and it must be passed the same count and slotv
// values. Making any other function call results in undefined behavior.
int slotsReserveSet(unsigned int count, unsigned int *slotv) __attribute__((nonnull (2)));

// This function's behavior is only defined when the prior function call was to
// slotsReserveSet with the same values for count, and the same values in the
// array slotv[0..count-1].
//
// If the given pid is already defined, then the behavior of this function is
// undefined.
//
// This registers the set of slots returned from slotsReserveSet as being
// associated with pid. To free these slots for use by a different process, call
// slotsRelease.
void slotsRegisterSet(pid_t pid, unsigned int count, unsigned int *slotv) __attribute__((nonnull (3)));


// This function's behavior is only defined when the prior function call was to
// slotsReserveSet with the same values for count, and the same values in the
// array slotv[0..count-1].
//
// The slots allocated by slotsReserveSet are freed by this function so that
// they may be used by other processes.
void slotsUnreserveSet(unsigned int count, unsigned int *slotv) __attribute__((nonnull (2)));

// "Unregisters" the given pid, and frees its slot for reuse
//
// The given pid must currently be registered.
void slotsRelease(pid_t pid);

#endif
