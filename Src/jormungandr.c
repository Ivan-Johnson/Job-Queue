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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"
#include "jormungandr.h"
#include "messenger.h"
#include "server.h"

#define OPTION_PRIORITY 'p'
#define OPTION_SLOTSMAX 's'
#define OPTION_SLOTSUSE 'c'
#define OPTION_PORT 'o'

#define DEFAULT_PORT 417784
#define DEFAULT_PORT_STR "417784"

const char *argp_program_version =
    "Jörmungandr v0.3.0-alpha\n"
    "Copyright(C) 2018, Ivan Tobias Johnson\n"
    "License GPLv2.0: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\n"
    "This software comes with no warranty, to the extent permitted by applicable law";
const char *argp_program_bug_address = "<git@IvanJohnson.net>";
static const char doc[] = "Jörmungandr -- a tool running a queue of jobs";
static const char args_doc[] =
    "launch <serverdir>\n" "schedule <serverdir> -- <cmd> [cmdargs...]";
static struct argp_option options[] = {
	{ "port", OPTION_PORT, "port", 0,
	 "When launching a server, specifies what port to communicate on (default: "
	 DEFAULT_PORT_STR ")", 0 },
	{ "slotsmax", OPTION_SLOTSMAX, "slots", 0,
	 "Specifies the number of slots to start servers with", 0 },
	{ "priority", OPTION_PRIORITY, 0, 0,
	 "Put the given command at the front of the queue", 0 },
	{ "slotsuse", OPTION_SLOTSUSE, "slots", 0,
	 "How many slots the given command occupies", 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

// Returns 0 on success, 1 on out of bounds, and other values for other
// problems.
static int parseUInt(char *string, unsigned int *out)
{
	char *end;
	long val = strtol(string, &end, 10);	//base 10
	*out = (unsigned int)val;
	if (end[0] != '\0') {
		return 2;
	}
	if (val < 0 || val > UINT_MAX) {
		return 1;
	}
	return 0;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	int fail;
	unsigned int val;

	switch (key) {
	case ARGP_KEY_INIT:
		//sentry values
		arguments->task = task_undefined;
		arguments->server = NULL;
		arguments->cmd = NULL;
		break;
	case OPTION_PRIORITY:
		arguments->priority = true;
		break;
	case OPTION_SLOTSUSE:
		fail = parseUInt(arg, &val);
		if (fail == 1) {
			printf
			    ("The number of slots must be in the range [%u, %u]\n",
			     0, UINT_MAX);
		}
		if (fail) {
			argp_usage(state);	// no return
		}
		arguments->slotsUse = val;
		break;
	case OPTION_SLOTSMAX:
		fail = parseUInt(arg, &val);
		if (fail == 1) {
			printf
			    ("The number of slots must be in the range [%u, %u]\n",
			     0, UINT_MAX);
		}
		if (fail) {
			argp_usage(state);	// no return
		}
		arguments->slotsMax = val;
		break;
	case OPTION_PORT:
		fail = parseUInt(arg, &val);
		if (fail == 1) {
			printf
			    ("The number of slots must be in the range [%u, %u]\n",
			     0, UINT_MAX);
		}
		if (fail) {
			argp_usage(state);	// no return
		}
		arguments->port = val;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) {
			if (strcmp(arg, "launch") == 0) {
				arguments->task = task_launch;
			} else if (strcmp(arg, "schedule") == 0) {
				arguments->task = task_schedule;
			} else {
				argp_usage(state);
			}
		} else if (state->arg_num == 1) {
			arguments->server = arg;
		} else {
			return ARGP_ERR_UNKNOWN;
		}
		break;
		// *INDENT-OFF*
	case ARGP_KEY_ARGS: {
		unsigned int argc = (unsigned int)(state->argc - state->next);
		arguments->cmd = malloc(sizeof(char *) * (argc + 1));
		assert(arguments->cmd);

		char **argv = state->argv + state->next;
		for (unsigned int i = 0; i < argc; i++) {
			arguments->cmd[i] = argv[i];
		}
		arguments->cmd[argc] = NULL;

		break;
	} case ARGP_KEY_END:
		// *INDENT-ON*
		switch (arguments->task) {
		case task_schedule:
			if (state->arg_num <= 2 || arguments->cmd == NULL) {
				argp_usage(state);
			}
			break;
		case task_launch:
			if (state->arg_num != 2 || arguments->server == NULL) {
				argp_usage(state);
			}
			break;
		default:
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

struct arguments parseArgs(int argc, char **argv)
{
	struct arguments args;

	// default values
	args.slotsMax = 0;
	args.slotsUse = 1;
	args.priority = false;
	args.port = DEFAULT_PORT;

	argp_parse(&argp, argc, argv, 0, 0, &args);
	return args;
}

int fulfilArgs(struct arguments args)
{
	int server = getServerDir(args.server);
	if (server < 0) {
		puts("Could not get serverdir");
		return 1;
	}

	int fail;
	struct job job;
	switch (args.task) {
	case task_launch:
		return messengerLaunchServer(server, args.slotsMax);
	case task_schedule:
		job.argv = args.cmd;
		job.priority = args.priority;
		job.slots = args.slotsUse;

		fail = messengerSendJob(server, job);
		if (fail) {
			fprintf(stderr, "Failed to send job");
			return 1;
		}
		return 0;
	case task_undefined:
		exit(1);
	default:
		exit(1);
	}
}

void freeArgs(struct arguments args)
{
	free(args.cmd);
}

#ifndef TEST
int main(int argc, char **argv)
{
	struct arguments args = parseArgs(argc, argv);
	int fail = fulfilArgs(args);
	freeArgs(args);
	return fail;
}
#endif
