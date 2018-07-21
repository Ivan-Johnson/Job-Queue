#ifndef FOO_H
#define FOO_H
/*
 * Src/jormungandr.h
 *
 * Parses and fulfils arguments.
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

struct arguments
{
	char *server;
	char *cmd;
};

struct arguments parseArgs(int argc, char **argv);
int fulfilArgs(struct arguments args);

#endif
