#!/bin/sh

mkdir /tmp/bwttmp

runInFileTest(){
	/usr/bin/time -f "%U %S" ./bwt -f -M $2 -i $1 -o /tmp/bwttmp/bwtout.tmp
	rm /tmp/bwttmp/bwtout.tmp
}

runTestsWithMemories(){
	runInFileTest $1 $(($2*1/4))  # 25% of input size
	runInFileTest $1 $(($2*2/4))  # 50% of input size
	runInFileTest $1 $(($2*3/4))  # 75% of input size
	runInFileTest $1 $(($2*1))    # 100% of input size
	runInFileTest $1 $(($2*5/4))  # 125% of input size
	runInFileTest $1 $(($2*6/4))  # 150% of input size
	runInFileTest $1 $(($2*8/4))  # 200% of input size
	runInFileTest $1 $(($2*10/4)) # 250% of input size
	runInFileTest $1 $(($2*12/4)) # 300% of input size
	runInFileTest $1 $(($2*16/4)) # 400% of input size
}

runTestsWithMemories genome10 10000000
runTestsWithMemories genome50 50000000
runTestsWithMemories genome100 100000000
runTestsWithMemories genome250 250000000
runTestsWithMemories genome500 500000000

runTestsWithMemories quote10 10000000
runTestsWithMemories quote50 50000000
runTestsWithMemories quote100 100000000
runTestsWithMemories quote250 250000000

runTestsWithMemories random10 10000000
runTestsWithMemories random50 50000000
runTestsWithMemories random100 100000000

runTestsWithMemories repeat10 10000000
runTestsWithMemories repeat50 50000000
runTestsWithMemories repeat100 100000000
