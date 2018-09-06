#!/bin/bash

# Tmp/demo_clientpart.bash
#
# Copyright(C) 2018, Ivan Tobias Johnson
#
# LICENSE: GPL 2.0

for x in {01..30}; do
	slots=$(((RANDOM%3)+1))
	time=$((20))
	echo "scheduling $x on $slots slots, for ~$time seconds"
	cmd='echo "starting '$x' on $CUDA_VISIBLE_DEVICES";
             sleep '$time';
             echo " finished '$x'"'
	./Bin/debug/Nontest/jormungandr schedule /tmp/Server/ \
		--slotsuse=$slots                             \
		-- /bin/bash -c "$cmd"
done

echo "waiting..."
sleep 3s

for x in {1..5}; do
	slots=$(((RANDOM%3)+1))
	time=$((RANDOM%15+4))
	echo "Prioritizing $x on $slots slots, for ~$time seconds"
	cmd='echo "PRIORITY: starting '$x' on $CUDA_VISIBLE_DEVICES";
             sleep '$time';
             echo " finished '$x'"'
	./Bin/debug/Nontest/jormungandr schedule /tmp/Server/ \
		--priority --slotsuse=$slots                  \
		-- /bin/bash -c "$cmd"
done
