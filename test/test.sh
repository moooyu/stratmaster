#!/bin/bash
pushd ../ > /dev/null
pushd src > /dev/null
make clean --quiet
make --quiet
cp strat.out ../test
popd > /dev/null
popd > /dev/null

rm -f err_log

echo "<----- Test is started. ----->"

for number in `seq 1 2` ; do 
	./strat.out < ./cases/case$number 2>log_stderr | tee log | grep YOU | awk '{ print $4, $5, $8, $11}' > tmp
	diff tmp ./cases/case1_out > difftmp
	pass=$?
	if [ $pass = "0" ]; then
		result="pass"
	else
# TODO: do another diff to check the correct error message by using log_stderr
		result="fail"
	fi
	echo "case $number is $result"

	if [ $pass != "0" ]; then
		cat ./cases/case$number | head -n 1
		echo "case $number" >> err_log
		cat log_stderr >> err_log
	fi

done

rm -f tmp
rm -f log_stderr
rm -f difftmp

echo "<----- Test is done. ----->"
echo 
echo "Check err_log file for further information"
