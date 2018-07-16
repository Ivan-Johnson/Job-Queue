/*
 * Test/test_foo.c
 *
 * A simple file for testing Src/foo.c
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: MIT License
 */
#include "unity.h"
#include "foo.h"

void setUp()
{
	//NOP
}

void tearDown()
{
	//NOP
}

void testZero()
{
	TEST_ASSERT_EQUAL_INT(0, getZero());
}

void testOne()
{
	TEST_ASSERT_EQUAL_INT(1, getOne());
}
