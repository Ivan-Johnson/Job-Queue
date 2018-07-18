/*
 * Test/testStack.c
 *
 * Tests the job stack
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "stack.h"

#include <unity.h>

void setUp()
{
	TEST_ASSERT_EQUAL_INT(0, stackSize());
}

void tearDown()
{
	stackPURGE();
	TEST_ASSERT_EQUAL_INT(0, stackSize());
}

void testWatchSize()
{
	struct job job;
	job.cmd = NULL;

	TEST_ASSERT_EQUAL_INT(0, stackSize());
	stackPush(job);
	TEST_ASSERT_EQUAL_INT(1, stackSize());
	stackPush(job);
	TEST_ASSERT_EQUAL_INT(2, stackSize());
	stackPush(job);
	TEST_ASSERT_EQUAL_INT(3, stackSize());
	stackPush(job);
	TEST_ASSERT_EQUAL_INT(4, stackSize());
	stackPush(job);
	TEST_ASSERT_EQUAL_INT(5, stackSize());
	stackPop();
	TEST_ASSERT_EQUAL_INT(4, stackSize());
	stackPop();
	TEST_ASSERT_EQUAL_INT(3, stackSize());
	stackPop();
	TEST_ASSERT_EQUAL_INT(2, stackSize());
	stackPop();
	TEST_ASSERT_EQUAL_INT(1, stackSize());
	stackPop();
	TEST_ASSERT_EQUAL_INT(0, stackSize());
}

void testStoredJobs()
{
	struct job job0, job1;
	job0.cmd = "iowelkjm,ncvxsd";
	job1.cmd = "oiuwelrkjmncxvw";

	stackPush(job0);
	stackPush(job1);

	TEST_ASSERT_TRUE(stackPeek().cmd == job1.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);

	TEST_ASSERT_TRUE(stackPeek().cmd == job0.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);
}
