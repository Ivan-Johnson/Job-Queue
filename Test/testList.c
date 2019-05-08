/*
 * Test/testStack.c
 *
 * Tests if tasklist can simultaneously function as both a stack and a
 * queue. Note that testQueue and testStack have more extensive tests of
 * tasklist.
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "tasklist.h"

#include <unity.h>
#include <stdbool.h>

char *args0[] = {"args_0", NULL};
struct job job0 = {1, false, args0};
char *args1[] = {"args1_0", "args1_1", "args1_2", NULL};
struct job job1 = {1, false, args1};
char *args2[] = {"args_0", NULL};
struct job job2 = {2, false, args2};
char *args3[] = {"args3_0", "args3_1", NULL};
struct job job3 = {1, false, args3};
char *args4[] = {"args4_0", "args4_1", "args4_2", "args4_3", "args4_4", NULL};
struct job job4 = {1, false, args4};
char *args5[] = {"args5_0", "args5_1", "args5_2", NULL};
struct job job5 = {3, false, args5};

// jobs array contains DISTINCT jobs.
#define ARR_LEN 5
struct job jobs[ARR_LEN];

void setUp()
{
	TEST_ASSERT_EQUAL_INT(0, listSize());

	jobs[0] = job1;
	jobs[1] = job2;
	jobs[2] = job3;
	jobs[3] = job4;
	jobs[4] = job5;
}

void tearDown()
{
	listFree();
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

void testRandomGrowth()
{
	struct job *arr = malloc(sizeof(struct job) * 1000);
	int old = 500;
	int new = 500;

	srand(56374729);
	for (int x = 0; x < 480; x++) {
		if (rand() % 4 != 0) {
			bool isPriority = rand() % 3;
			struct job job = jobs[rand() % ARR_LEN];
			listAdd(job, isPriority);

			if (isPriority) {
				old--;
				arr[old] = job;
			} else {
				arr[new] = job;
				new++;
			}
		} else {
			if (old != new) {
				struct job job = listNext();
				TEST_ASSERT_TRUE(jobEq(job, arr[old]));
				old++;
			}
		}
	}

	while (old != new) {
		struct job job = listNext();
		TEST_ASSERT_TRUE(jobEq(job, arr[old]));
		old++;
	}

	free(arr);
}

void testOverdraw()
{
	// Verify that calling listNext when list is empty returns JOB_ZEROS

	// Add and remove many items from the list. This is a roundabout way of
	// ensuring that the list's internal array does not contain the
	// JOB_ZEROS array.
	TEST_ASSERT_TRUE(listSize() == 0);
	size_t cap = listCurCapacity();
	for (size_t x = 0; x < cap; x++) {
		listAdd(jobs[0], false);
		listNext();
	}

	TEST_ASSERT_TRUE(listSize() == 0);
	TEST_ASSERT_TRUE(jobEq(listNext(), JOB_ZEROS));
}
