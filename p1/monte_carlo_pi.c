/**
  monte_carlo_pi.c

  Calculate the value of Pi using a Monte Carlo Simulation.

  To build: 
  make

  To run: 
  pmcpi <number of iterations> <random seed> <checkpoint interval>
  */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <prand.h>
#include <mpi.h>

//double report_cpu_time();
long long n, m, count;

int run_random_exeriments(long long int numPerProcess);

/** run_random_experiments:

  Throw random darts on the square [-1..1][-1..1] on the real number plane and
  count how many end up inside the circle centered at (0,0) with
  radius of 1.

return: the count of successful throws (as a double to allow larger counts)
*/
int run_random_experiments(long long int numPerProcess)
{
		double x, y;
		m = 0;						
		
		for (count =0; count< numPerProcess; count++) {
				x = -1 +  (2.0 * (random() / (RAND_MAX + 1.0)));
				y = -1 +  (2.0 * (random() / (RAND_MAX + 1.0)));
				/* check if (x,y) is within the unit circle */
				if ((x*x + y*y) < 1)  {
						m++;
				}
		}
		return m;
}


int main(int argc, char **argv)
{
		unsigned int seed;
		double pi;
		double startTime = 0, totalTime = 0;
		int myid, rem, nproc;
		MPI_Status status;
		int i;
		long long int numPerProcess = 0;
		int startingPoint =0;	
	
		if(argc < 3) {
				fprintf(stderr, "Usage: %s <number of iterations> <random seed>\n", argv[0]);
				exit(1);
		}

		n = atoll(argv[1]);
		seed = atoi(argv[2]);		

		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &nproc);
		MPI_Comm_rank(MPI_COMM_WORLD, &myid);	
		
		numPerProcess = n/nproc;
		rem = n % nproc;
		startingPoint = myid*numPerProcess;
		srandom(seed);
		if(myid == nproc-1){
			numPerProcess += rem;
		}
		startTime = MPI_Wtime();

		m = run_random_experiment(numPerProcess);
		if(myid == 0){
		  for(i = 1; i < nproc; i++){
			MPI_Recv(&count, 1, MPI_LONG_LONG_INT,i,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			m+=count;
		  }
		} else {
			MPI_Send(m, 1,MPI_LONG_LONG_INT, 0 ,MPI_ANY_TAG ,MPI_COMM_WORLD);
		}
	//	unrankRand(startingPoint*2);
			
		if (myid == 0){
		pi = (4*(double)m/n);
                    printf("p=%d;PI=%1.16lf;",nproc,pi);
                    totalTime = MPI_Wtime();
                    printf("TIME=%6.2lf;", (totalTime-startTime));
                    printf("iterations=%lld\n", n);
		}
		MPI_Finalize();
		exit(0);
}
