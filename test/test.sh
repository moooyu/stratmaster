#!/bin/bash
pushd ../
pushd src
make clean
make
cp strat.out ../test
popd
popd

for number in `seq 1` ; do 
	echo case$$number;
	./strat.out < ./test/case$number > tmp
	diff tmp ./test/case1_out
	rm tmp
done
