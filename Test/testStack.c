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

void testGrowth()
{
	struct job job0, job1, job2, job3, job4, job5;
	job0.cmd = "0";
	job1.cmd = "1";
	job2.cmd = "2";
	job3.cmd = "3";
	job4.cmd = "4";
	job5.cmd = "5";

	stackPush(job0);
	size_t cap = stackCurCapacity();
	for (size_t size = 1; size < cap - 2; size++) {
		stackPush(job0);
	}
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap - 2, stackSize());

	stackPush(job1);
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());

	stackPush(job2);
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap, stackSize());

	stackPush(job3);
	TEST_ASSERT_TRUE(stackCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 1, stackSize());

	stackPush(job4);
	stackPush(job5);
	TEST_ASSERT_TRUE(stackCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 3, stackSize());

	TEST_ASSERT_TRUE(stackPop().cmd == job5.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job4.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job3.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job2.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);
}
