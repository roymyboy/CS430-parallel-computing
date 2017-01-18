#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <prand.h>
#include <mpi.h>

/* 
 * Sequential bucketsort for randomly generated integers.
 *
 */

const int DEBUG_LEVEL = DEBUG;
int count_array_grows = 0;
int count_array_grows_parallel = 0;

double getMilliSeconds();
int compareTo(const void *, const void *);

const int TRUE = 1;
const int FALSE = 0;

int *A;
int *finalA;
int finalASize;
int **bucket; // bucket[i] = array holding the ith bucket
int **sBucket;
int **rBucket;
int *capacity; // capacity[i] = capacity of ith bucket
int *sCapacity;
int *size; // size[i] = next free location in ith bucket
int *sSize;
int *rSize;


void checkIfSorted(int *array, int n) {
	int i;
	int sorted;

	sorted = TRUE;
	for (i=0; i<n-1; i++) {
		if (array[i] > array[i+1]) {
				sorted = FALSE;
				break;
		}
    }

	if (sorted) {
		if (DEBUG_LEVEL >= 1) {
			fprintf(stderr, "array is sorted\n");
        }
	} else {
    	fprintf(stderr, "Error: array is not sorted!\n");
	}
}


void checkIfAllSorted(int myId, int numProcs) {
    int sCompareVal;
    int rCompareVal;
	MPI_Status status;

    sCompareVal = finalA[finalASize-1];

    if (myId == 0) {
        MPI_Send(&sCompareVal, 1, MPI_INT, 1, myId, MPI_COMM_WORLD);
    } else if (myId == numProcs-1) {
        MPI_Recv(&rCompareVal, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,     
                 &status);
    } else {
        MPI_Send(&sCompareVal, 1, MPI_INT, (myId + 1), myId, MPI_COMM_WORLD);
        MPI_Recv(&rCompareVal, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, 
                 &status);
    }     

 /*   if (myId != 0) {
        if (rCompareVal < finalA[finalASize-1]) {
            printf("All values in sorted array of process %d less than than all values\n"
                   "of sorted array in process %d.\n", myId-1, myId); 
        } else {
    	    fprintf(stderr, "Error: arrays are not sorted!\n");
        }
    }*/
}


/*
 * Generate n numbers using the given seed
 */
void generateInput(int *A, int myN, long int seed, int myId) {
	int i;
    long long int startingPosition = 0;;

	srandom(seed);
    if (myId == 0) {
        startingPosition = 0;
    } else {
        startingPosition = (myId * myN) - 1;
    }
    unrankRand(startingPosition);
    
	for (i=0; i<myN; i++) {
		A[i] = random();
	}
}


/*
 * Print the array, one element per line
 */
void printArray(int *finalA, int finalASize, int myId) {
	int i;

    for (i=0;i<finalASize;i++) {
 //       printf("Processor %d[%d]: %16d \n", myId, i, finalA[i]);
    }
}


/*
 *
 * Insert a given value into the specified bucket
 */
void insertInBucket(int value, int bucketIndex) {
	int *tmp;

	if (size[bucketIndex] == capacity[bucketIndex]) {
		//grow the bucket array
		tmp = (int *) malloc(sizeof(int)*(2*capacity[bucketIndex]));
		memcpy(tmp, bucket[bucketIndex], capacity[bucketIndex]*sizeof(int));
		free(bucket[bucketIndex]);
		bucket[bucketIndex] = tmp;
		capacity[bucketIndex] = 2 * capacity[bucketIndex];
		count_array_grows++;
		
        if (DEBUG_LEVEL >= 1) {
//			fprintf(stderr, "Growing bucket %d from %d to %d elements\n",
//					bucketIndex, capacity[bucketIndex]/2, capacity[bucketIndex]);
		}
	}

	bucket[bucketIndex][size[bucketIndex]] = value;
	size[bucketIndex]++;
}


void parInsertInBucket(int value, int bucketIndex) {
	int *tmp;

	if (sSize[bucketIndex] == sCapacity[bucketIndex]) {
		//grow the bucket array
		tmp = (int *) malloc(sizeof(int)*(2*sCapacity[bucketIndex]));
		memcpy(tmp, sBucket[bucketIndex], sCapacity[bucketIndex]*sizeof(int));
		free(sBucket[bucketIndex]);
		sBucket[bucketIndex] = tmp;
		sCapacity[bucketIndex] = 2 * sCapacity[bucketIndex];
		count_array_grows_parallel++;
		
        if (DEBUG_LEVEL >= 1) {
	//		fprintf(stderr, "Growing bucket %d from %d to %d elements\n",
	//				bucketIndex, sCapacity[bucketIndex]/2, sCapacity[bucketIndex]);
		}
	}

	sBucket[bucketIndex][sSize[bucketIndex]] = value;
	sSize[bucketIndex]++;
}


/*
 * compareTo function for using qsort
 * returns  -ve if *x < *y, 0 if *x == *y, +ve if *x > *y
 */
int compareTo(const void *x, const void *y) {
		return ((*(int *)x) - (*(int *)y));
}


/*
 * Sort indiviual bucket using quick sort from std C library
 */
void sortEachBucket(int numBuckets) {
	int i;

	for (i=0; i<numBuckets; i++) {
		qsort(bucket[i], size[i], sizeof(int), compareTo);
	}

	if (DEBUG_LEVEL >= 2) {
		for (i=0; i<numBuckets; i++) {
	//		fprintf(stderr, "bucket %d has %d elements\n", i, size[i]);
		}
    }
}


/* 
 * Combine all buckets back into the original array to finish the sorting
 *
 */
void combineBuckets(int *finalA, int finalASize, int numBuckets) {
	int i;
	int start = 0;

	for (i=0; i<numBuckets; i++) {
		memcpy(finalA+start, rBucket[i], sizeof(int)*rSize[i]);
		start = start + rSize[i];
	}
}


void finalCombineBuckets(int *finalA, int finalASize, int numBuckets) {
	int i;
	int start = 0;

	for (i=0; i<numBuckets; i++) {
		memcpy(finalA+start, bucket[i], sizeof(int)*size[i]);
		start = start + size[i];
		free(bucket[i]);
	}
	free(bucket);
}


/*
 * Use bucketsort to sort n uniformly distributed numbers in the range [0..2^31-1].
 * Input: int *A: array of ints A[0..n-1]
 *        int n: number of elements in the input array
 *        int numBuckets: number of buckets to use
 *
 */
void sequentialBucketsort(int *finalA, int finalASize, int numBuckets) {
	int share;
	int i;
	int bucketRange;
	int bucketIndex;

	share = finalASize / numBuckets;
	share = share + (share * 11)/100; // 11% extra for overflow

	capacity = (int *) malloc(sizeof(int)*numBuckets);
	size = (int *) malloc(sizeof(int)*numBuckets);
	bucket = (int **) malloc(sizeof(int *)* numBuckets);

	for (i=0; i<numBuckets; i++) {
		bucket[i] = (int *) malloc(sizeof(int)*share);
		capacity[i] = share;
		size[i] = 0;
	}

	bucketRange = RAND_MAX/numBuckets;

	for (i=0; i<finalASize; i++) {
		bucketIndex = finalA[i]/bucketRange;
		if (bucketIndex > numBuckets - 1)
				bucketIndex = numBuckets - 1;
		insertInBucket(finalA[i], bucketIndex);
	}

	sortEachBucket(numBuckets);
	finalCombineBuckets(finalA, finalASize, numBuckets);
	free(capacity);
	free(size);
}


void parallelBucketsort(int *A, int myN, int numProcs) {
	int share;
	int i;
	int bucketRange;
	int bucketIndex;

	share = myN / numProcs;
	share = share + (share * 11)/100; // 11% extra for overflow

	sCapacity = (int *) malloc(sizeof(int)*numProcs);
	sSize = (int *) malloc(sizeof(int)*numProcs);
	sBucket = (int **) malloc(sizeof(int *)* numProcs);

	for (i=0; i<numProcs; i++) {
		sBucket[i] = (int *) malloc(sizeof(int)*share);
		sCapacity[i] = share;
		sSize[i] = 0;
	}

	bucketRange = RAND_MAX/numProcs;

	for (i=0; i<myN; i++) {
		bucketIndex = A[i]/bucketRange;

		if (bucketIndex > numProcs - 1) {
				bucketIndex = numProcs - 1;
        }
		parInsertInBucket(A[i], bucketIndex);
	}
}


void sendBuckets(int myId, int numProcs) {
    int i,j;
    MPI_Status status;

    rSize = (int *) malloc (sizeof(int)*numProcs); 

    MPI_Alltoall(sSize, 1, MPI_INT, rSize, 1, MPI_INT, MPI_COMM_WORLD); 

    if (DEBUG_LEVEL >= 3) {
        for (i=0;i<numProcs;i++) {
            for (j=0;j<sSize[i];j++) {
                printf("Processor %d, sBucket[%d][%d]: %d\n", myId, i, j, sBucket[i][j]);
            }
        }
        for (i=0;i<numProcs;i++) {
            printf("Processor %d, size buffer recieved at index %d: %d\n", myId, i, rSize[i]); 
        }
    }
    
    rBucket = (int **) malloc(sizeof(int *)*numProcs);
    
    for (i=0;i<numProcs;i++) {
        rBucket[i] = (int *) malloc(sizeof(int)*rSize[i]);
    }
    
    if (DEBUG_LEVEL >= 3) {
        for (i=0;i<numProcs;i++) {
            printf("Processor %d, sSize[%d]: %d\n", myId, i, sSize[i]);
        }
        
        for (i=0;i<numProcs;i++) {
            printf("Processor %d, rSize[%d]: %d\n", myId, i, rSize[i]);
        }
    }
   
    for (i=0;i<numProcs;i++) {
        MPI_Sendrecv(sBucket[i], sSize[i], MPI_INT, i, 123, 
                     rBucket[i], rSize[i], MPI_INT, i, 123, MPI_COMM_WORLD, &status);       
    }

    finalASize = 0;
    
    for (i=0;i<numProcs;i++) {
        finalASize += rSize[i];
    }

	free(sCapacity);
	free(sSize);
}


void print_usage(char * program) {
		fprintf(stderr, "Usage %s <n, must be > 1> <#buckets, must between 1 and n>\
                <random seed>\n", program);
}


int main(int argc, char **argv) {
    int n, myN, myId, numProcs, numBuckets;
    unsigned int seed;
	double startTime, totalTime;
    
	if (argc != 4) {
		print_usage(argv[0]);
		exit(1);
	}
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myId);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

	n = atoi(argv[1]);
	numBuckets = atoi(argv[2]);
	seed = atoi(argv[3]);

	if ((numBuckets < 1) || (n < 1) || (n < numBuckets)) {
		print_usage(argv[0]);
		exit(1);
	}
			
    myN = n/numProcs;
    
    if (myId == numProcs-1) {
        myN += n % numProcs;
    }

	A = (int *) malloc(sizeof(int) * myN);

	generateInput(A, myN, seed, myId);

 	if (DEBUG_LEVEL >= 3) {
	    printArray(A, myN, myId);
    }

    startTime = MPI_Wtime();
    parallelBucketsort(A, myN, numProcs);
	sendBuckets(myId, numProcs);
    finalA = (int *) malloc(sizeof(int)*finalASize);
    combineBuckets(finalA, finalASize, numProcs);
	sequentialBucketsort(finalA, finalASize, numBuckets);
	totalTime = MPI_Wtime() - startTime;

	checkIfSorted(finalA, finalASize);

    if (numProcs != 1) {
        checkIfAllSorted(myId, numProcs);
    }

	if (DEBUG_LEVEL >= 1) {
		printf("Number of first-sorting array grows is %d\n", count_array_grows);
    }
   	if (DEBUG_LEVEL >= 1) {
		printf("Number of second-sorting array grows is %d\n", count_array_grows_parallel);
    }
 	if (DEBUG_LEVEL >= 3) {
 		printArray(A, n, myId);
    }
    printf("bucketsort: n = %d  m = %d buckets seed = %d time = %lf seconds\n",
	       n, numBuckets, seed,	totalTime);
    
    free(sBucket);
    free(sSize);
    free(rSize);
	free(A);
    free(finalA);
	exit(0);
}

/* vim: set ts=4: */
