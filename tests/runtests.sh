#!/bin/bash

function runtest {
	if ! test "$($1)" == "$2"; then
		echo Test failed '"'$1'"', got '"'$($1)'"' but expected '"'$2'"'
	fi
}

runtest "./test1" hi
runtest "echo ksjdlf" jj
