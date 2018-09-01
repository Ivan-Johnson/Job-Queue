#!/bin/bash

# Tmp/demo_clientpart.bash
#
# Copyright(C) 2018, Ivan Tobias Johnson
#
# LICENSE: GPL 2.0

for x in {1..30}; do
	slots=$(((RANDOM%3)+1))
	echo "scheduling $x for $slots slots"
	cmd='echo "        starting '$x' on $CUDA_VISIBLE_DEVICES"; sleep 0.3; echo "                finished '$x'"'
	./Bin/debug/Nontest/jormungandr schedule /tmp/Server/ --slotsuse=$slots \
		-- /bin/bash -c "$cmd"
done
