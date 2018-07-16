#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    MPI_Init(NULL, NULL);

    int num_proc;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int proc_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    srand(time(NULL) + proc_rank);

    printf("Process %i, pid = %i\n", proc_rank, getpid());

    if (proc_rank == 0) {
        printf("There are %i processes\n", num_proc);
        // This process waits for the others and prints all their messages!
        int data;
        for (size_t i = 0; i < num_proc - 1; i++) {
            printf("Waiting for a message...\n");
            MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Received message from process %i\n", data);
        }

    } else {
        int data;
        data = proc_rank;
        int time = rand() % 10 + 1;
        printf("Process %i sleeping %i seconds...\n", proc_rank, time);
        sleep(time);
        printf("Process %i finished sleeping\n", proc_rank);
        MPI_Send(&data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    printf("Process %i finalizing\n", proc_rank);

    MPI_Finalize();

    printf("Process %i finished\n", proc_rank);

    return 0;
}