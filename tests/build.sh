#!/bin/bash

for file in test*.c; do
	if ! gcc $file -o ${file/.c/}; then
		break
	fi
done
