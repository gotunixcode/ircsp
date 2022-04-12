#!/bin/sh


echo "Total number of HEADER files."
find ./ -type f -name "*.h" | wc -l

echo "Total number of SOURCE files."
find ./ -type f -name "*.c" | wc -l

echo "Counting lines of code in source files"
#cat include/*.h include/*/*.h source/*.c source/*/*.c source/*/*/*.c| wc -l
find ./ -type f -name "*.c" -exec cat {} \; | wc -l

echo "Counting lines of code in header files"
find ./ -type f -name "*.h" -exec cat {} \; | wc -l

