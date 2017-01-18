#PROGRAMMING ASSIGNMENT 1: Monte Carlo Simulation
..*Utsav Roy
..*CS430

##OVERVIEW 
The main purpose of this file is to parallelize sequential program to enable faster calculation.
Monte Carlo simulation to calculate compute the value of PI was parallelized in this assignment.

##INCLUDE FILES:
	*Makefile 
	*monte_carlo_pi.c
	*smcpi.pbs
	*timing.c
	*README.md - this file

##COMPILING AND USING:
The program was compiled using:
	$make 

to run the program
	$ mpiexec -nN pmcpi <number of iterations> <random number seed>

In this program user will be prompted to pass in argument inorder to run the program.
  
