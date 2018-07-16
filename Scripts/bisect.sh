#!/bin/sh

#Scripts/bisect.sh
#
#A git bisect run script
#
#Copyright(C) 2018, Ivan Tobias Johnson
#
#LICENSE: MIT License

#if build fails, return 125 so that this commit is skipped
make IS_TEST=yes bisect_make || exit 125
make IS_TEST=yes bisect_run
