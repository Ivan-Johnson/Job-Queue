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

void setUp()
{
	//NOP
}

void tearDown()
{
	//NOP
}

char buf[1000];
const size_t bufLen = sizeof(buf) / sizeof(char);

/*
 * Verifies that job serialization uses correct amount of storage
 */
void testUsage()
{
	struct job j;
	ssize_t tmp;

	j.cmd = "Hello, World!";
	tmp = serializeJob(j, buf, bufLen);
	TEST_ASSERT_EQUAL_INT(strlen(j.cmd) + 1, tmp);

	j.cmd = "";
	tmp = serializeJob(j, buf, bufLen);
	TEST_ASSERT_EQUAL_INT(strlen(j.cmd) + 1, tmp);

	j.cmd = "a";
	tmp = serializeJob(j, buf, bufLen);
	TEST_ASSERT_EQUAL_INT(strlen(j.cmd) + 1, tmp);
}

/*
 * Verify that serialization fails when the given buffer is too small
 */
void testTooSmall()
{
	struct job j;
	ssize_t tmp;
	j.cmd = "a";

	char buf[1];
	size_t bufLen = sizeof(buf) / sizeof(char);
	tmp = serializeJob(j, buf, bufLen);
	TEST_ASSERT_EQUAL_INT(-1, tmp);
}

/*
 * Ensure that the serializing then unserializing a job results in a job
 * equivalent to the first.
 */
void testIdentity()
{
	struct job jIn;
	struct job jOut;
	ssize_t tmp;

	jIn.cmd = "Hello, World!";
	tmp = serializeJob(jIn, buf, bufLen);
	TEST_ASSERT_TRUE(tmp >= 0);
	unserializeJob(&jOut, buf, (size_t) tmp);
	TEST_ASSERT_EQUAL_INT(0, strcmp(jIn.cmd, jOut.cmd));
}
