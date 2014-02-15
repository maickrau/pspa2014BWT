#!/bin/sh

mkdir /tmp/bwttmp

runTest(){
	/usr/bin/time -f "%U %S" ./bwt -f -M $2 -i $1 -o /tmp/bwttmp/bwtout.tmp
	rm /tmp/bwttmp/bwtout.tmp
	/usr/bin/time -f "%U %S" ./bwt -m -i $1 -o /tmp/bwttmp/bwtout.tmp
	rm /tmp/bwttmp/bwtout.tmp
}

runTest genome50 150000000
runTest quote50 150000000
runTest random50 150000000
runTest repeat50 150000000
