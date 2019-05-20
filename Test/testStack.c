/*
 * Test/testStack.c
 *
 * Tests if joblist can function as a stack
 *
 * Copyright(C) 2018-2019, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "joblist.h"

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
	listInitialize();

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

void testWatchSize()
{
	struct job job;

	TEST_ASSERT_EQUAL_INT(0, listSize());
	listAdd(job, true);
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listAdd(job, true);
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listAdd(job, true);
	TEST_ASSERT_EQUAL_INT(3, listSize());
	listAdd(job, true);
	TEST_ASSERT_EQUAL_INT(4, listSize());
	listAdd(job, true);
	TEST_ASSERT_EQUAL_INT(5, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(4, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(3, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(2, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(1, listSize());
	listNext();
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

void testPeek()
{
	listAdd(job0, true);
	listAdd(job1, true);

	TEST_ASSERT_TRUE(jobEq(listPeek(), job1));
	listNext();

	TEST_ASSERT_TRUE(jobEq(listPeek(), job0));
}

void testPopSimple()
{
	listAdd(job0, true);
	listAdd(job1, true);

	TEST_ASSERT_TRUE(jobEq(listNext(), job1));
	TEST_ASSERT_TRUE(jobEq(listNext(), job0));
}

void testPop()
{
	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, listSize());

	listAdd(job0, true);
	listAdd(job1, true);
	listAdd(job2, true);
	listAdd(job3, true);
	listAdd(job4, true);

	//state: job0, job1, job2, job3, job4
	TEST_ASSERT_EQUAL_INT(5, listSize());

	TEST_ASSERT_TRUE(jobEq(listNext(), job4));
	TEST_ASSERT_TRUE(jobEq(listNext(), job3));

	//state: job0, job1, job2
	TEST_ASSERT_EQUAL_INT(3, listSize());

	listAdd(job4, true);
	listAdd(job3, true);

	//state: job0, job1, job2, job4, job3
	TEST_ASSERT_EQUAL_INT(5, listSize());

	TEST_ASSERT_TRUE(jobEq(listNext(), job3));
	TEST_ASSERT_TRUE(jobEq(listNext(), job4));
	TEST_ASSERT_TRUE(jobEq(listNext(), job2));
	TEST_ASSERT_TRUE(jobEq(listNext(), job1));
	TEST_ASSERT_TRUE(jobEq(listNext(), job0));

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, listSize());

	listAdd(job2, true);
	listAdd(job0, true);

	//state: job2, job0
	TEST_ASSERT_EQUAL_INT(2, listSize());

	TEST_ASSERT_TRUE(jobEq(listNext(), job0));
	TEST_ASSERT_TRUE(jobEq(listNext(), job2));

	//state: ∅
	TEST_ASSERT_EQUAL_INT(0, listSize());
}

void testGrowth()
{
	listAdd(job0, true);
	size_t cap = listCurCapacity();
	for (size_t size = 1; size < cap - 2; size++) {
		listAdd(job0, true);
	}
	TEST_ASSERT_EQUAL_INT(cap, listCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap - 2, listSize());

	//state: job0 x cap - 2

	listAdd(job1, true);
	TEST_ASSERT_EQUAL_INT(cap, listCurCapacity());

	listAdd(job2, true);
	TEST_ASSERT_EQUAL_INT(cap, listCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap, listSize());

	//state: job0 x cap - 2, job1, job2
	//at this point, the stack is filled to capacity

	listAdd(job3, true);
	TEST_ASSERT_TRUE(listCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 1, listSize());

	//state: job0 x cap - 2, job1, job2, job3
	//at this point, the stack has just grown

	listAdd(job4, true);
	listAdd(job5, true);
	//state: job0 x cap - 2, job1, job2, job3, job4, job5
	TEST_ASSERT_TRUE(listCurCapacity() > cap);
	TEST_ASSERT_EQUAL_INT(cap + 3, listSize());

	//state: job0 x cap - 2, job1, job2, job3, job4, job5
	//pop to ensure that jobs 1-3 weren't lost in the growth process
	TEST_ASSERT_TRUE(jobEq(listNext(), job5));
	TEST_ASSERT_TRUE(jobEq(listNext(), job4));
	TEST_ASSERT_TRUE(jobEq(listNext(), job3));
	TEST_ASSERT_TRUE(jobEq(listNext(), job2));
	TEST_ASSERT_TRUE(jobEq(listNext(), job1));
	TEST_ASSERT_TRUE(jobEq(listNext(), job0));
}

void testShrinkage()
{
	listAdd(job0, true);
	size_t cap = listCurCapacity();
	//If we grow once, we can shrink by popping the item at shrinkpoint
	size_t shrinkpoint = cap / 2;
	for (size_t size = 1; size <= cap; size++) {
		if (shrinkpoint - 2 <= size && size <= shrinkpoint + 2) {
			listAdd(jobs[size - shrinkpoint + 2], true);
		} else {
			listAdd(job0, true);
		}
	}

	//state: job0, …, job0, job1, job2, job3, job4, job5, job0, …, job0
	//The last push made the stack grow, and job3 is at the shrinkpoint
	TEST_ASSERT_TRUE(cap < listCurCapacity());
	TEST_ASSERT_EQUAL_INT(cap + 1, listSize());

	//popping immediately after growing should not cause a shrink
	listNext();
	TEST_ASSERT_TRUE(cap < listCurCapacity());

	for (size_t size = cap; size > shrinkpoint + 3; size--) {
		listNext();
	}
	TEST_ASSERT_EQUAL_INT(shrinkpoint + 3, listSize());


	//confirm that our data is still intact
	TEST_ASSERT_TRUE(jobEq(listNext(), job5));
	TEST_ASSERT_TRUE(jobEq(listNext(), job4));

	//confirm that we haven't shrunk yet
	TEST_ASSERT_EQUAL_INT(shrinkpoint + 1, listSize());
	TEST_ASSERT_TRUE(cap < listCurCapacity());

	//shrink
	TEST_ASSERT_TRUE(jobEq(listNext(), job3));
	TEST_ASSERT_EQUAL_INT(shrinkpoint, listSize());
	TEST_ASSERT_EQUAL_INT(cap, listCurCapacity());

	//confirm that the top of the stack is still intact
	TEST_ASSERT_TRUE(jobEq(listNext(), job2));
	TEST_ASSERT_TRUE(jobEq(listNext(), job1));
}
