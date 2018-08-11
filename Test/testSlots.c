/*
 * Test/testSlots.c
 *
 * Tests the slots module
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "slots.h"

#include <assert.h>
#include <stdbool.h>
#include <unity.h>

#define NUM_SLOTS 2
#define NUM_PIDS  5

struct pid {
	unsigned int numSlots;
	unsigned int slots[NUM_SLOTS];
};

struct pid pids[NUM_PIDS];

void setUp()
{
	int code = slotsMalloc(NUM_SLOTS);
	TEST_ASSERT_EQUAL_INT(0, code);

	for (int a = 0; a < NUM_PIDS; a++) {
		pids[a].numSlots = 0;
	}
}

void tearDown()
{
	slotsFree();
}

/*
 * Verify that no slot is being reused
 */
void validate()
{
	bool slotused[NUM_SLOTS];
	for (int x = 0; x < NUM_SLOTS; x++) {
		slotused[x] = false;
	}

	for (unsigned int a = 0; a < NUM_PIDS; a++) {
		for (unsigned int b = 0; b < pids[a].numSlots; b++) {
			unsigned int slot = pids[a].slots[b];
			TEST_ASSERT_TRUE(slot < NUM_SLOTS);
			TEST_ASSERT_FALSE(slotused[slot]);
			slotused[slot] = true;
		}
	}
}

void register_(pid_t pid, unsigned int numSlots)
{
	assert(numSlots > 0 && numSlots <= NUM_SLOTS);
	pids[pid].numSlots = numSlots;
	for (unsigned int a = 0; a < numSlots; a++) {
		pids[pid].slots[a] = NUM_SLOTS;
	}
	int code = slotsRegister(pid, numSlots, pids[pid].slots);
	TEST_ASSERT_EQUAL_INT(0, code);
	validate();
}

void release(pid_t pid)
{
	slotsRelease(pid);
	pids[pid].numSlots = 0;
}

/*
 * A simple test where only valid operations are performed, no process needs
 * more than one slot, and no pids are reused
 */
void test_basic()
{
	assert(NUM_PIDS >= 5);
	assert(NUM_SLOTS == 2);

	// a: ∅, b: ∅
	register_(0, 1);
	// a: 0, b: ∅
	release(0);
	// a: ∅, b: ∅
	register_(1, 1);
	// a: 1, b: ∅
	register_(2, 1);
	// a: 1, b: 2
	release(1);
	// a: ∅, b: 2
	register_(3, 1);
	// a: 3, b: 2
	release(3);
	// a: ∅, b: 2
	release(2);
	// a: ∅, b: ∅
	register_(4, 1);
	// a: 4, b: ∅
	release(4);
}

/*
 * A simple test where only valid operations are performed and no pids are
 * reused. Some processes use multiple slots.
 */
void test_2slot()
{
	assert(NUM_PIDS >= 4);
	assert(NUM_SLOTS == 2);

	// a: ∅, b: ∅
	register_(0, 1);
	// a: 0, b: ∅
	register_(1, 1);
	// a: 0, b: 1
	release(1);
	// a: 0, b: ∅
	release(0);
	// a: ∅, b: ∅
	register_(2, 2);
	// a: 2, b: 2
	release(2);
	// a: ∅, b: ∅
	register_(3, 2);
	// a: 3, b: 3
	release(3);
	// a: ∅, b: ∅
}

/*
 * A simple test where only valid operations are performed and no process needs
 * more than one slot. Some pids are reused.
 */
void test_pidreuse()
{
	assert(NUM_PIDS >= 5);
	assert(NUM_SLOTS == 2);

	// a: ∅, b: ∅
	register_(0, 1);
	// a: 0, b: ∅
	release(0);
	// a: ∅, b: ∅
	register_(1, 1);
	// a: 1, b: ∅
	register_(0, 1);
	// a: 1, b: 0
	release(0);
	release(1);
	// a: ∅, b: ∅
}

/*
 * Try registering when slotv is null.
 */
void test_nullslotv()
{
	assert(NUM_PIDS >= 5);
	assert(NUM_SLOTS == 2);

	// a: ∅, b: ∅
	TEST_ASSERT_EQUAL_INT(0, slotsRegister(0, 1, NULL));
	// a: 0, b: ∅
	slotsRelease(0);
	// a: ∅, b: ∅
	TEST_ASSERT_EQUAL_INT(0, slotsRegister(1, 1, NULL));
	// a: 1, b: ∅
	TEST_ASSERT_EQUAL_INT(0, slotsRegister(2, 1, NULL));
	// a: 1, b: 2
	slotsRelease(1);
	// a: ∅, b: 2
	TEST_ASSERT_EQUAL_INT(0, slotsRegister(3, 1, NULL));
	// a: 3, b: 2
	slotsRelease(3);
	// a: ∅, b: 2
	slotsRelease(2);
	// a: ∅, b: ∅
	TEST_ASSERT_EQUAL_INT(0, slotsRegister(4, 1, NULL));
	// a: 4, b: ∅
	slotsRelease(4);
}
