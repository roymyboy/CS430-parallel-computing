##NAME: Jiahang Li and Utsav Roy
##DATE: 24/10/2016
##ASSIGNMENT: Project 2 Parallel BucketSort
##CS430 - Parallel Computing 

#BUILDING AND RUNNING 

To compile this program
	$make 

To run this program
	$mpiexec -n <number of processor> ./executable <n, must be> 1> <#buckets, must between 1 and n> <random seed>

This program can also run with included .sh script.
To run the program with script

	$ test1.sh 
or
	$ test2.sh
