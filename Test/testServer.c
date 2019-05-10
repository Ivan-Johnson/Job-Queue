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

// TODO make a private header so that we don't have to forward declare like
// this?
int serverOpen(int dirFD, unsigned int numSlots, unsigned int port);
void serverClose();

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

	fail = serverOpen(sFD, 3, port);
	TEST_ASSERT_EQUAL_INT(0, fail);

	int foundPort = serverGetPort(sFD);
	serverClose();

	TEST_ASSERT_TRUE(foundPort > 0);
	TEST_ASSERT_EQUAL_INT(port, foundPort);
}
