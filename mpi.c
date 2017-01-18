/* hello world in MPI */
/* appropriate header file */
#include <mpi.h>
#include <asm/param.h> /* for MAXHOSTNAMELEN */

int main (int argc, char **argv) {

	int pid;
	int nproc;
	char hostname[MAXHOSTNAMELEN];
	MPI_Status *status;
	char* msgtag = "Hello I am the message";	

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	
	if(pid == 0){
		int x = 12345;
		MPI_Send(&x,1 ,MPI_INT,1 ,msgtag, MPI_COMM_WORLD);
	} else if (pid == 1){
		int x;
		MPI_Recv(&x,1 , MPI_INT, 0, msgtag, MPI_COMM_WORLD, &status);
	}

	gethostname(hostname, 100);
	printf("HEllo! I am process:%d of size:%d running on %s. \n", pid, nproc, hostname);

	MPI_Finalize();
	exit(0);

}
