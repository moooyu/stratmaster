#!/bin/bash
pushd ../ > /dev/null
pushd src > /dev/null
make clean --quiet
make --quiet
cp strat.out ../test
popd > /dev/null
popd > /dev/null

for number in `seq 1` ; do 
	./strat.out < ./cases/case$number 2>/dev/null | tee log | grep YOU | awk '{ print $4, $5, $8, $11}' > tmp
	diff tmp ./cases/case1_out
	if [ "$?" = "0" ]; then
		result="pass"
	else
		result="fail"
	fi
	rm tmp
	echo "case $number is $result"

done
