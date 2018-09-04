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
	TEST_ASSERT_EQUAL_INT(0, stackSize());

	jobs[0] = job1;
	jobs[1] = job2;
	jobs[2] = job3;
	jobs[3] = job4;
	jobs[4] = job5;
}

void tearDown()
{
	stackFree();
	TEST_ASSERT_EQUAL_INT(0, stackSize());
}

void testWatchSize()
{
	struct job job;

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
	stackPush(job0);
	stackPush(job1);

	TEST_ASSERT_TRUE(jobEq(stackPeek(), job1));
	stackPop();

	TEST_ASSERT_TRUE(jobEq(stackPeek(), job0));
}

void testPopSimple()
{
	stackPush(job0);
	stackPush(job1);

	TEST_ASSERT_TRUE(jobEq(stackPop(), job1));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job0));
}

void testPop()
{
	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());

	stackPush(job0);
	stackPush(job1);
	stackPush(job2);
	stackPush(job3);
	stackPush(job4);

	//state: job0, job1, job2, job3, job4
	TEST_ASSERT_EQUAL_INT(5, stackSize());

	TEST_ASSERT_TRUE(jobEq(stackPop(), job4));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job3));

	//state: job0, job1, job2
	TEST_ASSERT_EQUAL_INT(3, stackSize());

	stackPush(job4);
	stackPush(job3);

	//state: job0, job1, job2, job4, job3
	TEST_ASSERT_EQUAL_INT(5, stackSize());

	TEST_ASSERT_TRUE(jobEq(stackPop(), job3));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job4));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job2));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job1));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job0));

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());

	stackPush(job2);
	stackPush(job0);

	//state: job2, job0
	TEST_ASSERT_EQUAL_INT(2, stackSize());

	TEST_ASSERT_TRUE(jobEq(stackPop(), job0));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job2));

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, stackSize());
}

void testGrowth()
{
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
	TEST_ASSERT_TRUE(jobEq(stackPop(), job5));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job4));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job3));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job2));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job1));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job0));
}

void testShrinkage()
{
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
	TEST_ASSERT_TRUE(jobEq(stackPop(), job5));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job4));

	//confirm that we haven't shrunk yet
	TEST_ASSERT_EQUAL_INT(shrinkpoint + 1, stackSize());
	TEST_ASSERT_TRUE(cap < stackCurCapacity());

	//shrink
	TEST_ASSERT_TRUE(jobEq(stackPop(), job3));
	TEST_ASSERT_EQUAL_INT(shrinkpoint, stackSize());
	TEST_ASSERT_EQUAL_INT(cap, stackCurCapacity());

	//confirm that the top of the stack is still intact
	TEST_ASSERT_TRUE(jobEq(stackPop(), job2));
	TEST_ASSERT_TRUE(jobEq(stackPop(), job1));
}
