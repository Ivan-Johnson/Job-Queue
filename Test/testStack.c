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
	stackFree();
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

void testPeek()
{
	struct job job0, job1;
	job0.cmd = "iowelkjm,ncvxsd";
	job1.cmd = "oiuwelrkjmncxvw";

	stackPush(job0);
	stackPush(job1);

	TEST_ASSERT_TRUE(stackPeek().cmd == job1.cmd);
	stackPop();

	TEST_ASSERT_TRUE(stackPeek().cmd == job0.cmd);
}

void testPopSimple()
{
	struct job job0, job1;
	job0.cmd = "iowelkjm,ncvxsd";
	job1.cmd = "oiuwelrkjmncxvw";

	stackPush(job0);
	stackPush(job1);

	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);
}

void testPop()
{
	struct job job0, job1, job2, job3, job4;
	job0.cmd = "0";
	job1.cmd = "1";
	job2.cmd = "2";
	job3.cmd = "3";
	job4.cmd = "4";

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());

	stackPush(job0);
	stackPush(job1);
	stackPush(job2);
	stackPush(job3);
	stackPush(job4);

	//state: job0, job1, job2, job3, job4
	TEST_ASSERT_EQUAL_INT(5, stackSize());

	TEST_ASSERT_TRUE(stackPop().cmd == job4.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job3.cmd);

	//state: job0, job1, job2
	TEST_ASSERT_EQUAL_INT(3, stackSize());

	stackPush(job4);
	stackPush(job3);

	//state: job0, job1, job2, job4, job3
	TEST_ASSERT_EQUAL_INT(5, stackSize());

	TEST_ASSERT_TRUE(stackPop().cmd == job3.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job4.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job2.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());

	stackPush(job2);
	stackPush(job0);

	//state: job2, job0
	TEST_ASSERT_EQUAL_INT(2, stackSize());

	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job2.cmd);

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());
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

	//state: job0 x cap - 2

	stackPush(job1);
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());

	stackPush(job2);
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap, stackSize());

	//state: job0 x cap - 2, job1, job2
	//at this point, the stack is filled to capacity

	stackPush(job3);
	TEST_ASSERT_TRUE(stackCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 1, stackSize());

	//state: job0 x cap - 2, job1, job2, job3
	//at this point, the stack has just grown

	stackPush(job4);
	stackPush(job5);
	//state: job0 x cap - 2, job1, job2, job3, job4, job5
	TEST_ASSERT_TRUE(stackCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 3, stackSize());

	//state: job0 x cap - 2, job1, job2, job3, job4, job5
	//pop to ensure that jobs 1-3 weren't lost in the growth process
	TEST_ASSERT_TRUE(stackPop().cmd == job5.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job4.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job3.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job2.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job0.cmd);
}

void testShrinkage()
{
	struct job job0, job1, job2, job3, job4, job5;
	job0.cmd = "0";
	job1.cmd = "1";
	job2.cmd = "2";
	job3.cmd = "3";
	job4.cmd = "4";
	job5.cmd = "5";
	struct job jobs[] = {job1, job2, job3, job4, job5};

	stackPush(job0);
	size_t cap = stackCurCapacity();
	//If we grow once, we can shrink by popping the item at shrinkpoint
	size_t shrinkpoint = cap / 2;
	for (size_t size = 1; size <= cap; size++) {
		if (shrinkpoint - 2 <= size && size <= shrinkpoint + 2) {
			stackPush(jobs[size - shrinkpoint + 2]);
		} else {
			stackPush(job0);
		}
	}

	//state: job0, …, job0, job1, job2, job3, job4, job5, job0, …, job0
	//The last push made the stack grow, and job3 is at the shrinkpoint
	TEST_ASSERT_TRUE(cap < stackCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap + 1, stackSize());

	//popping immediately after growing should not cause a shrink
	stackPop();
	TEST_ASSERT_TRUE(cap < stackCurCapacity());

	for (size_t size = cap; size > shrinkpoint + 3; size--) {
		stackPop();
	}
	TEST_ASSERT_EQUAL_INT(shrinkpoint + 3, stackSize());


	//confirm that our data is still intact
	TEST_ASSERT_TRUE(stackPop().cmd == job5.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job4.cmd);

	//confirm that we haven't shrunk yet
	TEST_ASSERT_EQUAL_INT(shrinkpoint + 1, stackSize());
	TEST_ASSERT_TRUE(cap < stackCurCapacity());

	//shrink
	TEST_ASSERT_TRUE(stackPop().cmd == job3.cmd);
	TEST_ASSERT_EQUAL_INT(shrinkpoint, stackSize());
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());

	//confirm that the top of the stack is still intact
	TEST_ASSERT_TRUE(stackPop().cmd == job2.cmd);
	TEST_ASSERT_TRUE(stackPop().cmd == job1.cmd);
}
