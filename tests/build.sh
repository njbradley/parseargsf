#!/bin/bash

for file in test*.c; do
	if ! gcc $file -o ${file/.c/} -I../; then
		break
	fi
done
