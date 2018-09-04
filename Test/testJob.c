/*
 * Test/testJob.c
 *
 * Tests job.c
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include "job.h"

#include <string.h>
#include <sys/types.h>
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
	jobs[0] = job1;
	jobs[1] = job2;
	jobs[2] = job3;
	jobs[3] = job4;
	jobs[4] = job5;
}

void tearDown()
{
	//NOP
}

char buf[1000];
const size_t bufLen = sizeof(buf) / sizeof(char);

/*
 * Verify that serialization fails when the given buffer is too small
 */
void testTooSmall()
{
	struct job j;

	j.slots = 1;
	char *argv[] = {"a", NULL};
	j.argv = argv;

	char buf[1];
	size_t bufLen = sizeof(buf) / sizeof(char);
	ssize_t tmp = serializeJob(j, buf, bufLen);
	TEST_ASSERT_EQUAL_INT(-1, tmp);
}

/*
 * Ensure that the serializing then unserializing a job results in a job
 * equivalent to the first.
 */
void testIdentity()
{
	char buf[150];
	size_t bufLen = sizeof(buf) / sizeof(buf[0]);

	for (int x = 0; x < ARR_LEN; x++) {
		struct job jIn = jobs[x];
		struct job jOut;

		ssize_t used = serializeJob(jIn, buf, bufLen);
		TEST_ASSERT_TRUE(used >= 0);
		char *end;
		int fail = unserializeJob(&jOut, buf, bufLen, &end);
		TEST_ASSERT_FALSE(fail);
		TEST_ASSERT_TRUE(used == end - buf);
		TEST_ASSERT_TRUE(jobEq(jIn, jOut));

		freeUnserializedJob(jOut);
	}
}

void testEq()
{
	for (int x = 0; x < ARR_LEN; x++) {
		struct job jA = jobs[x];
		for (int y = 0; y < ARR_LEN; y++) {
			struct job jB = jobs[y];
			TEST_ASSERT_EQUAL(x==y, jobEq(jA, jB));
		}
	}
}

void testClone()
{
	for (int x = 0; x < ARR_LEN; x++) {
		struct job job;
		int fail = cloneJob(&job, jobs[x]);
		TEST_ASSERT_FALSE(fail);
		TEST_ASSERT_TRUE(jobEq(jobs[x], job));

		// verify that cloneJob didn't just copy the pointers
		TEST_ASSERT_NOT_EQUAL(jobs[x].argv, job.argv);
		int i = 0;
		while (jobs[x].argv[i] != NULL) {
			TEST_ASSERT_NOT_EQUAL(jobs[x].argv[i], job.argv[i]);
			i++;
		}
		TEST_ASSERT_NULL(job.argv[i]);
		freeJobClone(job);
	}
}
