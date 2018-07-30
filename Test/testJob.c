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

char *args0[] = {"args0_0"};
struct job job0 = {false, sizeof(args0) / sizeof(char*), args0};
char *args1[] = {"args1_0", "args1_1", "args1_2"};
struct job job1 = {false, sizeof(args1) / sizeof(char*), args1};
char *args2[] = {"args2_0"};
struct job job2 = {false, sizeof(args2) / sizeof(char*), args2};
char *args3[] = {"args3_0", "args3_1"};
struct job job3 = {false, sizeof(args3) / sizeof(char*), args3};
char *args4[] = {"args4_0", "args4_1", "args4_2", "args4_3", "args4_4"};
struct job job4 = {false, sizeof(args4) / sizeof(char*), args4};
char *args5[] = {"args5_0", "args5_1", "args5_2"};
struct job job5 = {false, sizeof(args5) / sizeof(char*), args5};

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

	j.argc = 1;
	char *argv[] = {"a"};
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

		ssize_t tmp = serializeJob(jIn, buf, bufLen);
		TEST_ASSERT_TRUE(tmp >= 0);
		tmp = unserializeJob(&jOut, buf, (size_t) tmp);
		TEST_ASSERT_TRUE(tmp == 0);
		TEST_ASSERT_TRUE(jobEq(jIn, jOut) == 0);

		freeUnserializedJob(jOut);
	}
}
