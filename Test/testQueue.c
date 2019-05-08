/*
 * Test/testQueue.c
 *
 * Tests if tasklist can function as a queue.
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "tasklist.h"

#include <unity.h>

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
	listInitialize();

	TEST_ASSERT_EQUAL_INT(0, listSize());
	//advances the list indicies to near the end of the length 128 array
	for (int x = 0; x < 125; x++) {
		listAdd(job0, false);
		listNext();
	}
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
}

/*
 * Test the list's ability to track its size while performing trivial
 * operations
 */
void testSize0()
{
	TEST_ASSERT_EQUAL_INT(0, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(3, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(0, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listAdd(job0, false);
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

/*
 * Test queue peeking for trivial operations
 */
void testPeek0()
{
	for (size_t x = 0; x < ARR_LEN; x++) {
		listAdd(jobs[x], false);
		TEST_ASSERT_TRUE(jobEq(listPeek(), jobs[0]));
	}
	TEST_ASSERT_EQUAL_INT(ARR_LEN, listSize());
	for (size_t x = 0; x < ARR_LEN; x++) {
		TEST_ASSERT_TRUE(jobEq(listPeek(), jobs[x]));
		listNext();
	}
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

/*
 * Test trivial dequeue operations
 */
void testDequeue()
{
	for (size_t x = 0; x < ARR_LEN; x++) {
		listAdd(jobs[x], false);
	}
	TEST_ASSERT_EQUAL_INT(ARR_LEN, listSize());
	for (size_t x = 0; x < ARR_LEN; x++) {
		TEST_ASSERT_TRUE(jobEq(listNext(), jobs[x]));
	}
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

/*
 * Ensure that the queue grows properly
 */
void testGrow()
{
	TEST_ASSERT_EQUAL_INT(0, listSize());
	size_t cap = listCurCapacity();

#define JOBC 300
	TEST_ASSERT_TRUE(JOBC > cap + 5);
	struct job jobs[JOBC];
	for(unsigned int x = 0; x < JOBC; x++) {
		jobs[x].argv = NULL;
		jobs[x].priority = false;
		jobs[x].slots = x;
	}

	for (size_t x = 0; x < cap - 2; x++) {
		listAdd(jobs[x], false);
	}

	listAdd(jobs[cap-2], false);
	listAdd(jobs[cap-1], false);

	//Queue is now at capacity
	TEST_ASSERT_EQUAL_INT(cap, listSize());
	TEST_ASSERT_EQUAL_INT(cap, listCurCapacity());
	//grow the queue
	listAdd(jobs[cap], false);
	TEST_ASSERT_TRUE(cap < listCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap+1, listSize());

	listAdd(jobs[cap+1], false);
	listAdd(jobs[cap+2], false);
	listAdd(jobs[cap+3], false);
	//confirm that the queue waits before shrinking
	for(size_t x = 0; x < 10; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], listNext()));
		TEST_ASSERT_TRUE(cap < listCurCapacity());
	}
	//dequeue until we get to
	for(size_t x = 10; x <= cap + 3; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], listNext()));
	}
	TEST_ASSERT_EQUAL_INT(0, listSize());
#undef JOBC
}

/*
 * Ensure that the queue shrinks properly
 */
void testShrinkage()
{
	TEST_ASSERT_EQUAL_INT(0, listSize());
	size_t cap = listCurCapacity();

#define JOBC 300
	TEST_ASSERT_TRUE(JOBC > cap + 5);
	struct job jobs[JOBC];
	for(unsigned int x = 0; x < JOBC; x++) {
		jobs[x].argv = NULL;
		jobs[x].priority = false;
		jobs[x].slots = x;
	}

	for (size_t x = 0; x <= cap; x++) {
		listAdd(jobs[x], false);
	}
	TEST_ASSERT_TRUE(cap < listCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap+1, listSize());

	for (size_t x = 0; x <= cap; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], listNext()));
	}
	TEST_ASSERT_EQUAL_INT(0, listSize());
	// NOTE: the queue's default size is it's minimum size;
	// so even though it's not storying anything right now,
	// it should not have shrunk below it's origional size.
	TEST_ASSERT_TRUE(cap == listCurCapacity());
#undef JOBC
}
