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

void setUp()
{
	struct job job;
	job.cmd = NULL;

	TEST_ASSERT_EQUAL_INT(0, queueSize());
	//advances the queue indicies to near the end of the length 128 array
	for (int x = 0; x < 125; x++) {
		queueEnqueue(job);
		queueDequeue();
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
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
	struct job job;
	job.cmd = NULL;

	TEST_ASSERT_EQUAL_INT(0, queueSize());
	queueEnqueue(job);
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job);
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job);
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueEnqueue(job);
	TEST_ASSERT_EQUAL_INT(3, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(2, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueDequeue();
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	queueEnqueue(job);
	TEST_ASSERT_EQUAL_INT(1, queueSize());
	queueEnqueue(job);
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
	struct job job0, job1, job2, job3, job4;
	job0.cmd = "0";
	job1.cmd = "1";
	job2.cmd = "2";
	job3.cmd = "3";
	job4.cmd = "4";
	struct job jobs[] = {job0, job1, job2, job3, job4};
	size_t jobc = 5;

	for (size_t x = 0; x < jobc; x++) {
		queueEnqueue(jobs[x]);
		TEST_ASSERT_TRUE(queuePeek().cmd == jobs[0].cmd);
	}
	TEST_ASSERT_EQUAL_INT(jobc, queueSize());
	for (size_t x = 0; x < jobc; x++) {
		TEST_ASSERT_TRUE(queuePeek().cmd == jobs[x].cmd);
		queueDequeue();
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}

/*
 * Test trivial dequeue operations
 */
void testDequeue()
{
	struct job job0, job1, job2, job3, job4;
	job0.cmd = "0";
	job1.cmd = "1";
	job2.cmd = "2";
	job3.cmd = "3";
	job4.cmd = "4";
	struct job jobs[] = {job0, job1, job2, job3, job4};
	size_t jobc = 5;

	for (size_t x = 0; x < jobc; x++) {
		queueEnqueue(jobs[x]);
	}
	TEST_ASSERT_EQUAL_INT(jobc, queueSize());
	for (size_t x = 0; x < jobc; x++) {
		TEST_ASSERT_TRUE(queueDequeue().cmd == jobs[x].cmd);
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
		jobs[x].cmd = malloc(sizeof(char));
		TEST_ASSERT_TRUE_MESSAGE(jobs[x].cmd,
					"We ran out of memory while testing?");
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
		TEST_ASSERT_TRUE(jobs[x].cmd == queueDequeue().cmd);
		TEST_ASSERT_TRUE(cap < queueCurCapacity());
	}
	//dequeue until we get to
	for(size_t x = 10; x <= cap + 3; x++) {
		TEST_ASSERT_TRUE(jobs[x].cmd == queueDequeue().cmd);
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());

	for(size_t x = 0; x < JOBC; x++) {
		free(jobs[x].cmd);
	}
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
		jobs[x].cmd = malloc(sizeof(char));
		TEST_ASSERT_TRUE_MESSAGE(jobs[x].cmd,
					"We ran out of memory while testing?");
	}

	for (size_t x = 0; x <= cap; x++) {
		queueEnqueue(jobs[x]);
	}
	TEST_ASSERT_TRUE(cap < queueCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap+1, queueSize());

	for (size_t x = 0; x <= cap; x++) {
		TEST_ASSERT_TRUE(jobs[x].cmd == queueDequeue().cmd);
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
	// NOTE: the queue's default size is it's minimum size;
	// so even though it's not storying anything right now,
	// it should not have shrunk below it's origional size.
	TEST_ASSERT_TRUE(cap == queueCurCapacity());

	for(size_t x = 0; x < JOBC; x++) {
		free(jobs[x].cmd);
	}
#undef JOBC
}
