#!/bin/bash
killall ./Bin/debug/Nontest/jormungandr > /dev/null 2>&1
rm -rf /tmp/Server
./Bin/debug/Nontest/jormungandr launch /tmp/Server -s 3
for x in {1..30}; do
	sleep 1.1
	slots=$(((RANDOM%3)+1))
	echo "scheduling $x for $slots slots"
	cmd='echo "        starting '$x' on $CUDA_VISIBLE_DEVICES"; sleep 10; echo "                finished '$x'"'
	./Bin/debug/Nontest/jormungandr schedule /tmp/Server/ --slotsuse=$slots \
		-- /bin/bash -c "$cmd"
done
