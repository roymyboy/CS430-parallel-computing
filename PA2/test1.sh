#!/bin/bash

for num in 20000000 100000000 200000000 500000000
do
		for numbuckets in 40000
		do
		   mpiexec -n 2 ./parallel-bucketsort $num  $numbuckets 12345
		done
done
