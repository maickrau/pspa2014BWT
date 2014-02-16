#!/bin/sh

#make sure bwt is compiled with -g -pg before running

mkdir /tmp/bwttmp

runTest(){
	/usr/bin/time -f "%U %S" ./bwt -f -M $2 -i $1 -o /tmp/bwttmp/bwtout.tmp
	gprof bwt gmon.out > $3
	rm /tmp/bwttmp/bwtout.tmp
}

runTest genome50 12500000 25_1.out
runTest genome50 12500000 25_2.out
runTest genome50 12500000 25_3.out
runTest genome50 12500000 25_4.out

runTest genome50 100000000 200_1.out
runTest genome50 100000000 200_2.out
runTest genome50 100000000 200_3.out
runTest genome50 100000000 200_4.out
