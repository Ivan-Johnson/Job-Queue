/*
 * Src/jormungandr.c
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <argp.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "messenger.h"
#include "jormungandr.h"

const char *argp_program_version = "\
Jörmungandr v0.1.0-alpha\n\
Copyright(C) 2018, Ivan Tobias Johnson\n\
License GPLv2.0: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\n\
This software comes with no warranty, to the extent permitted by applicable law";
const char *argp_program_bug_address = "<git@IvanJohnson.net>";
static char doc[] = "Jörmungandr -- a tool running a queue of jobs";
static char args_doc[] = "server [command]";

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key)
	{
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) {
			arguments->server = arg;
		} else if (state->arg_num == 1) {
			arguments->cmd = arg;
		} else {
			return ARGP_ERR_UNKNOWN;
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num == 0) {
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	//TODO: best way of adding support for running command with arguments?
	//https://www.gnu.org/software/libc/manual/html_node/Argp-Special-Keys.html#Argp-Special-Keys
	//ARGP_KEY_ARGS
	return 0;
}

static struct argp argp = { 0, parse_opt, args_doc, doc, 0, 0, 0 };

struct arguments parseArgs(int argc, char **argv)
{
	struct arguments args;
	args.server = NULL;
	args.cmd = NULL;
	argp_parse (&argp, argc, argv, 0, 0, &args);
	return args;
}

int fulfilArgs(struct arguments args)
{
	printf("Server is %s\n", args.server);
	printf("cmd is %s\n", args.cmd);

	int status;
	struct server server;
	status = messengerGetServer(args.server, &server);
	if (status) {
		puts("Error when getting server");
		return 1;
	}

	if (args.cmd == NULL) {
		return 0;
	}
	struct job job;
	job.cmd = args.cmd;
	return messengerSendJob(server, job);
}

#ifndef TEST
int main(int argc, char **argv)
{
	struct arguments args = parseArgs(argc, argv);
	return fulfilArgs(args);
}
#endif
