#include <mpi.h>
#include <stdio.h>

#define N_ITEMS 16

int main(int argc, char const* argv[]) {
    int num_proc;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int global[N_ITEMS];

    for (size_t i = 0; i < N_ITEMS; i++) {
        global[i] = i;
    }

    int local[N_ITEMS];

    int sendcounts[4] = {1, 2, 3, 10};
    int displs[4] = {0, 1, 3, 6};

    // al 0 le llega 1 int; al 1, 2; al 2, 3; al 3, 10; empezando en las posiciones en displs

    MPI_Scatterv(global, sendcounts, displs, MPI_INT, local, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    char str[256];
    int c = 0;
    char* p = str;
    for (size_t i = 0; i < sendcounts[rank]; i++) {
        p += c;
        c = sprintf(p, "%i\t", local[i]);
    }

    printf("[%i]\t %s\n", rank, str);

    MPI_Finalize();
    return 0;
}
