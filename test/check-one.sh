#!/bin/bash

time test/run-one.sh $1 || exit 1
echo
md5 tmp/*.fits > tmp/checksum-$1 || exit 1
cmp test/checksum tmp/checksum-$1 || exit 1

