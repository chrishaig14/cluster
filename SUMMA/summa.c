#include <mpi.h>
#include <stdio.h>

#define N_ITEMS 16

int main(int argc, char const* argv[]) {
    int num_proc;

    MPI_Init(NULL,NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int array[N_ITEMS];

    for (size_t i = 0; i < N_ITEMS; i++) {
        array[i] = i;
    }

    int local[N_ITEMS];

    MPI_Scatter(array, N_ITEMS / num_proc, MPI_INT, local, N_ITEMS / num_proc, MPI_INT, 0,
                MPI_COMM_WORLD);

    char str [256];
    int c = 0;
    char* p = str;
    for (size_t i = 0; i < N_ITEMS / num_proc; i++) {
        p += c;
        c = sprintf(p, "%i\t", local[i]);
    }

    printf("[%i]\t %s\n", rank, str);

    MPI_Finalize();
    return 0;
}
