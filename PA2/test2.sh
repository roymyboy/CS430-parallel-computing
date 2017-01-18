#!/bin/bash

numbuckets=50000
for n in 50000000 100000000 200000000 500000000 
do
	for seed in 6764 445455 876533
	do
		echo
		mpiexec -n 4 ./parallel-bucketsort $n $numbuckets $seed
	done
done
