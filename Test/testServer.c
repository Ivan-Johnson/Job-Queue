/*
 * Test/testServer.c
 *
 * Tests the server
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <assert.h>
#include <stdlib.h>
#include <unity.h>

#include "server.h"

#define SDIR "/tmp/JormungandrServerTest/"

int sFD;
struct server s;

void setUp()
{
	// NOP
}

void tearDown()
{
	// TODO: use C instead, if reasonable
	int fail = system("rm -rf " SDIR);
	assert(!fail);
}

void testPortFile()
{
	unsigned int port = 7462; // completely arbitrary
	int fail;

	// TODO: don't use the methods we're testing in the setup -_-
	int sFD = getServerDir(SDIR);
	assert(sFD >= 0);

	struct server s;
	fail = openServer(sFD, &s, 3, port);
	TEST_ASSERT_EQUAL_INT(0, fail);

	unsigned int foundPort = serverGetPort(sFD);
	serverClose(s);

	TEST_ASSERT_EQUAL_INT(port, foundPort);
}
