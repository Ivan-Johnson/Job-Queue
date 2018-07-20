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
