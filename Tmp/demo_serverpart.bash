#!/bin/bash

# Tmp/demo_serverpart.bash
#
# Copyright(C) 2018, Ivan Tobias Johnson
#
# LICENSE: GPL 2.0

killall jormungandr > /dev/null 2>&1
rm -rf /tmp/Server
make
./Bin/debug/Nontest/jormungandr launch /tmp/Server -s 3
