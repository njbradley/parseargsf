#!/bin/bash

for file in test*.c; do
	gcc $file -o ${file/.c/}
done
