/*
 * Test/test_bar.c
 *
 * A simple file for testing Src/bar.c
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: MIT License
 */
#include "unity.h"
#include "bar.h"

void setUp()
{
	//NOP
}

void tearDown()
{
	//NOP
}

void testA()
{
	TEST_ASSERT_EQUAL_INT('A', getA());
}

void testB()
{
	TEST_ASSERT_EQUAL_INT('B', getB());
}
