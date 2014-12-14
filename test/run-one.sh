#!/bin/bash

for i in 0 1 2 3 5 10 15 25 50; do
    echo -n .
    $1/mf2d $i example/test-1.fits !tmp/med-1-$i.fits !tmp/dif-1-$i.fits || exit 1
done
echo
