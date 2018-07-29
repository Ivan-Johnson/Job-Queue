#ifndef JOB_H
#define JOB_H
/*
 * Src/job.h
 *
 * The job struct stores the information need to run a particular command
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

struct job {
	// index 0 is the command we run. Indicies [1, argc] are arguments
	char *cmd;
};


#endif
