/*
 * Test/testQueue.c
 *
 * Tests the job queue
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "queue.h"

#include <unity.h>

char *args0[] = {"args_0"};
struct job job0 = {1, false, sizeof(args0) / sizeof(char*), args0};
char *args1[] = {"args1_0", "args1_1", "args1_2"};
struct job job1 = {1, false, sizeof(args1) / sizeof(char*), args1};
char *args2[] = {"args_0"};
struct job job2 = {2, false, sizeof(args2) / sizeof(char*), args2};
char *args3[] = {"args3_0", "args3_1"};
struct job job3 = {1, false, sizeof(args3) / sizeof(char*), args3};
char *args4[] = {"args4_0", "args4_1", "args4_2", "args4_3", "args4_4"};
struct job job4 = {1, false, sizeof(args4) / sizeof(char*), args4};
char *args5[] = {"args5_0", "args5_1", "args5_2"};
struct job job5 = {3, false, sizeof(args5) / sizeof(char*), args5};

// jobs array contains DISTINCT jobs.
#define ARR_LEN 5
struct job jobs[ARR_LEN];

void setUp()
{
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	//advances the queue indicies to near the end of the length 128 array
	for (int x = 0; x < 125; x++) {
		queueEnqueue(job0);
		queueDequeue();
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());

	jobs[0] = job1;
	jobs[1] = job2;
	jobs[2] = job3;
	jobs[3] = job4;
	jobs[4] = job5;
}

void tearDown()
{
	queueFree();
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}

/*
 * Test the queue's ability to track its size while performing trivial
 * operations
 */
void testSize0()
{
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(3, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job0);
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}

/*
 * Test queue peeking for trivial operations
 */
void testPeek0()
{
	for (size_t x = 0; x < ARR_LEN; x++) {
		queueEnqueue(jobs[x]);
		TEST_ASSERT_TRUE(jobEq(queuePeek(), jobs[0]));
	}
	TEST_ASSERT_EQUAL_INT(ARR_LEN, queueSize());
	for (size_t x = 0; x < ARR_LEN; x++) {
		TEST_ASSERT_TRUE(jobEq(queuePeek(), jobs[x]));
		queueDequeue();
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}

/*
 * Test trivial dequeue operations
 */
void testDequeue()
{
	for (size_t x = 0; x < ARR_LEN; x++) {
		queueEnqueue(jobs[x]);
	}
	TEST_ASSERT_EQUAL_INT(ARR_LEN, queueSize());
	for (size_t x = 0; x < ARR_LEN; x++) {
		TEST_ASSERT_TRUE(jobEq(queueDequeue(), jobs[x]));
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}

/*
 * Ensure that the queue grows properly
 */
void testGrow()
{
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	size_t cap = queueCurCapacity();

#define JOBC 300
	TEST_ASSERT_TRUE(JOBC > cap + 5);
	struct job jobs[JOBC];
	for(int x = 0; x < JOBC; x++) {
		jobs[x].argv = NULL;
		jobs[x].priority = false;
		jobs[x].argc = x;
	}

	for (size_t x = 0; x < cap - 2; x++) {
		queueEnqueue(jobs[x]);
	}

	queueEnqueue(jobs[cap-2]);
	queueEnqueue(jobs[cap-1]);

	//Queue is now at capacity
	TEST_ASSERT_EQUAL_INT(cap, queueSize());
	TEST_ASSERT_EQUAL_INT(cap, queueCurCapacity());
	//grow the queue
	queueEnqueue(jobs[cap]);
	TEST_ASSERT_TRUE(cap < queueCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap+1, queueSize());

	queueEnqueue(jobs[cap+1]);
	queueEnqueue(jobs[cap+2]);
	queueEnqueue(jobs[cap+3]);
	//confirm that the queue waits before shrinking
	for(size_t x = 0; x < 10; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], queueDequeue()));
		TEST_ASSERT_TRUE(cap < queueCurCapacity());
	}
	//dequeue until we get to
	for(size_t x = 10; x <= cap + 3; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], queueDequeue()));
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
#undef JOBC
}

/*
 * Ensure that the queue shrinks properly
 */
void testShrinkage()
{
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	size_t cap = queueCurCapacity();

#define JOBC 300
	TEST_ASSERT_TRUE(JOBC > cap + 5);
	struct job jobs[JOBC];
	for(int x = 0; x < JOBC; x++) {
		jobs[x].argv = NULL;
		jobs[x].priority = false;
		jobs[x].argc = x;
	}

	for (size_t x = 0; x <= cap; x++) {
		queueEnqueue(jobs[x]);
	}
	TEST_ASSERT_TRUE(cap < queueCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap+1, queueSize());

	for (size_t x = 0; x <= cap; x++) {
		TEST_ASSERT_TRUE(jobEq(jobs[x], queueDequeue()));
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	// NOTE: the queue's default size is it's minimum size;
	// so even though it's not storying anything right now,
	// it should not have shrunk below it's origional size.
	TEST_ASSERT_TRUE(cap == queueCurCapacity());
#undef JOBC
}
