#!/bin/bash

#Scripts/sumarize.bash
#
#Summarize the results of Unity tests
#
#Copyright(C) 2018, Ivan Tobias Johnson
#
#LICENSE: MIT License



TMPFILE="$(mktemp)"

file_fail_count=0
file_ignore_count=0
file_count=0

total_tests=0
total_fails=0
total_ignores=0

for x in $* ; do
	x_summary=$(cat $x | tail -n 2 | head -n 1)

	num_tests=$(cut -d ' ' -f 1 <<< "$x_summary")
	((total_tests += num_tests))
	num_fails=$(cut -d ' ' -f 3 <<< "$x_summary")
	((total_fails += num_fails))
	num_ignores=$(cut -d ' ' -f 5 <<< "$x_summary")
	((total_ignores += num_ignores))
	test_name=$(basename $x)

	#TODO: don't hardcode spacing. Can whiptail format tables? printf?
	#printf "%25s│%10s│%9s│%s\n" "$test_name" "$num_fails" "$num_ignores" "$num_tests"
	echo "$test_name|$num_fails|$num_ignores|$num_tests" >> "$TMPFILE"

	((file_count++))
	if [ "$num_fails" -gt 0 ]; then
		((file_fail_count++))
	elif [ "$num_ignores" -gt 0 ]; then
		((file_ignore_count++))
	fi
done

cat << EOF
#########################
#SUMMARIZED TEST RESULTS#
#########################

EOF

echo "TOTAL|$total_fails|$total_ignores|$total_tests" >> "$TMPFILE"
column --table --table-columns "GROUP NAME,FAILURES,IGNORED,TEST COUNT" --separator '|' --output-separator ' │ ' "$TMPFILE"
rm "$TMPFILE"

echo "
$file_fail_count groups had at least one failure
$file_ignore_count passed with one or more ignored test cases
$file_count groups in total"

[ "$total_fails" -eq 0 ]
exit $?
