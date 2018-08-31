#!/bin/bash
set -euo pipefail
dir="$(mktemp -d)"
pushd "$dir"

response="$(curl https://api.github.com/repos/ThrowTheSwitch/Unity/releases/latest)"
tag="$(echo "$response" | python3 -c "import sys, json; print(json.load(sys.stdin)['tag_name'])")"

tag_url="https://raw.githubusercontent.com/ThrowTheSwitch/Unity/$tag"

wget "$tag_url/src/unity.c"
wget "$tag_url/src/unity.h"
wget "$tag_url/src/unity_internals.h"

RUBY_SCRIPTS="auto/generate_test_runner.rb auto/type_sanitizer.rb"
RUBY="/bin/ruby"
for x in $RUBY_SCRIPTS; do
	wget "$tag_url/$x"
	x=$(basename $x)
	sed -i.origional "1s;^;#!$RUBY\n;" generate_test_runner.rb
	sudo install $x /usr/local/bin
done

gcc -c -Wall -Werror -fpic unity.c -o unity.o
gcc -shared -o libunity.so unity.o
sudo install libunity.so /usr/local/lib/

sudo install unity.h unity_internals.h /usr/local/include/

echo "NOTE: you *might* need to run the following commands as root"
echo 'echo /usr/local/lib/ > /etc/ld.so.conf.d/usr_local.conf'
echo 'ldconfig'
echo
echo "Also, make sure that /usr/local/bin appears in your $PATH"

popd
rm -r "$dir"
