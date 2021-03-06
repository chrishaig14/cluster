#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include "DMBR.h"


int main(int argc, char** argv) {

	if (argc != 3) {
        printf("Usage: mult matrix_a matrix_b result");
        return 0;
    }

	MPI_Init(NULL, NULL);

	int proc_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
	int num_proc;
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	if (num_proc < 2) {
		fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	decomposition_columns(proc_rank,num_proc,argv[1],argv[2]);

	MPI_Finalize();

	return 0;

}