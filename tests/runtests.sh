#!/bin/bash

function assertoutput {
	if ! test "$($1)" == "$2"; then
		echo
		echo Test failed '"'$1'"', got '"'$($1)'"' but expected '"'$2'"'
		echo
	else
		echo passed test $1
	fi
}

function assertwork {
	if ! $1 > /dev/null; then
		echo
		echo Test failed '"'$1'"', returned error code
		echo
	else
		echo passed test $1
	fi
}

function assertfail {
	if $1 2> /dev/null; then
		echo
		echo Test failed '"'$1'"', didnt returned error code
		echo
	else
		echo passed test $1
	fi
}

assertoutput "./test1 5 5" 25
assertoutput "./test2 hihi hoho" hihihoho
assertoutput "./test3" ""
assertoutput "./test3 --func1" "func1"
assertoutput "./test3 --func2" "func2"
assertoutput "./test4 1/2/3 4:5:6" "1127 14706"

assertfail "./test3 -func1"
assertwork "./test5 %%66"
assertfail "./test5 %6"
assertfail "./test5 %%%6"
