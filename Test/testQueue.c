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
 * tests the queue's ability to track it's own size when it is wrapped around
 * the circular array
 */
void testSize1()
{
	struct job job;
	job.cmd = NULL;

	TEST_ASSERT_EQUAL_INT(0, queueSize());
	//advances the queue indicies to near the end of the length 128 array
	for (int x = 0; x < 120; x++) {
		queueEnqueue(job);
		queueDequeue();
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());

	//advance the new side of the queue past the end of the array
	for (int len = 0; len < 20; len++) {
		queueEnqueue(job);
		TEST_ASSERT_EQUAL_INT(len + 1, queueSize());
	}

	//advance the old side of the queue past the end of the array
	for (int len = 20; len > 0; len--) {
		queueDequeue(job);
		TEST_ASSERT_EQUAL_INT(len - 1, queueSize());
	}
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
		TEST_ASSERT_TRUE(queuePeek().cmd == jobs[x].cmd);
	}
	TEST_ASSERT_EQUAL_INT(jobc, queueSize());
	for (size_t x = jobc - 1; x != SIZE_MAX; x--) { // deliberate underflow
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
	for (size_t x = jobc - 1; x != SIZE_MAX; x--) { // deliberate underflow
		TEST_ASSERT_TRUE(queueDequeue().cmd == jobs[x].cmd);
	}
	TEST_ASSERT_EQUAL_INT(0, queueSize());
}
