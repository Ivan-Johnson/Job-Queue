/*
 * Src/foo.c
 *
 * Simple file to help test the tests
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: MIT License
 */

#include <stdio.h>
#include <stdlib.h>

#include "foo.h"

int getZero()
{
	return 0;
}

int getOne()
{
	return 1;
}

#ifndef TEST
int main()
{
	puts("Hello, World!");
	return 0;
}
#endif
