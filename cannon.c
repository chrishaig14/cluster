#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MATRIX_BLOCK 1
#define BLOCK_SIZE 2

int** create_empty_matrix(int n) {
    int** a = (int**)malloc(n * sizeof(int*));

    for (size_t i = 0; i < n; i++) {
        a[i] = (int*)malloc(n * sizeof(int));
        for( size_t j =0; j < n; j++)
        {
            a[i][j] = 0;
        }
    }

    return a;
}

int** create_random_matrix(int n) {
    int** a = create_empty_matrix(n);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            a[i][j] = rand() % 1000000;
        }
    }

    return a;
}

void delete_matrix(int** a, int n) {
    for (size_t i = 0; i < n; i++) {
        free(a[i]);
    }
    free(a);
}

void send_matrix_block(int** a, int n, int dest) 
{
    for (size_t i = 0; i < n; i++) {
        MPI_Send(a[i], n, MPI_INT, dest, MATRIX_BLOCK, MPI_COMM_WORLD);
    }
}

void receive_matrix_block(int n, int from, int** dest) 
{
    for (size_t i = 0; i < (*n); i++) {
        MPI_Recv(dest[i], (*n), MPI_INT, from, MATRIX_BLOCK, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }
}

void matrix_mult(int** a, int** b, int n, int** c) 
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            c[i][j] = 0;
            for (size_t k = 0; k < n; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void sum_matrix_on_place(int** dest, int** toAdd, int n)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            dest[i][j] += toAdd[i][j];
        }
    }
}

void reset_matrix(int** matrix, int n)
{
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            dest[i][j] = 0;
        }
    }
}

void copy_matrix_part(int** a, 
    int start_row, int start_col, 
    int dest_row, int dest_col, 
    int n_copy,
    int** dest)
{
    for( int i = 0; i < n; i++)
    {
        for( int j = 0; j < n; j++ )
        {
             dest[dest_row + i][ dest_col + j] = a[i + start_row][j + start_col];
        }
    }
}

void copy_matrix_part(int** a, 
    int start_row, int start_col, 
    int n_copy,
    int** dest)
{
    copy_matrix_part(a, start_row, start_col, 0, 0, n_copy, dest);
}

void intialize_cannon_blocks(int amount_blocks, int iterations, int n_block, 
            int** a, int** b, int** a_proccess, int** b_procces )
{
    copy_matrix_part(a, 0, 0, n_block, a_proccess);
    copy_matrix_part(b, 0, 0, n_block, b_proccess);
    
    int** sub_a = create_empty_matrix(n_block);
    int** sub_b = create_empty_matrix(n_block);

    int block_row, block_column, shift_row, shift_col;
    for( int i = 1; i < amount_blocks; i++) 
    {
        block_row = i / iterations;
        block_column = i % iterations;
        shift_row =( block_row - i ) % iterations;
        shift_col =( block_column - i ) % iterations;
        
        copy_matrix_part(a, block_row*n_block, shift_col*n_block, n_block, sub_a);
        copy_matrix_part(b, shift_row*n_block, block_column*n_block, n_block, sub_b);

        MPI_Send((void*)block_size, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD);
        send_matrix_block(sub_a, block_size, i);
        send_matrix_block(sub_b, block_size, i);

        reset_matrix(sub_a);
        reset_matrix(sub_b);
    }
    delete_matrix(sub_a);
    delete_matrix(sub_b);
}

int main(int argc, char** argv) {
    // Inicializar MPI
    MPI_Init(NULL, NULL);

    // Obtener el numero de procesos
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Obtener el rank (id) del proceso actual
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    //TODO SEARCH MINIMUM SQUARE TO USE PROCCESS
    double root = sqrt(world_size);
    double floor_root = floor(root);
    if( root - floor_root != 0 )
    {
        printf("Can't run Cannon algorithms with a number of processes that does not have a square root");
        return 1;
    }

    int iterations = (int)root;
    int block_size; 
    int** sub_matrix_a, 
    int** sub_matrix_b, 
    int** sub_matrix_c;
    if( world_rank == 0 )
    {
        
        int n = rand() % 4999 + 1;
        if( iterations > n )
        {
            //RESOLVE LOCALLY
            //SEND DO NOT RESOLVE
            return 0;
        } 
        //Makes it possible to always calculate the block size
        n += n % iterations;
        block_size = n / iterations;
        printf("MATRIX SIZE = %i\n", n);
        //START multiplying matrixes
        int** a = create_random_matrix(n);
        int** b = create_random_matrix(n);
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        intialize_cannon_blocks(n, iterations, block_size, a, b, sub_matrix_a, sub_matrix_b);
        delete_matrix(a);
        delete_matrix(b);
    } else 
    {
        //RECIEVE INITIAL DATA FOR CANNON
        MPI_Recv(&block_size, 1, MPI_INT, 0, BLOCK_SIZE, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        receive_matrix_block(sub_matrix_a, block_size, 0);
        receive_matrix_block(sub_matrix_b, block_size, 0);
    }

    //DO CANNON
    sub_matrix_c = create_empty_matrix(block_size);
    int** multiplication = create_empty_matrix(block_size);
    int proccess_row = world_rank / iterations;
    int proccess_column = world_rank % iterations;
    int next_a_col = ( proccess_column - 1 ) % iterations;
    int next_b_row = ( world_rank - 1 ) % iterations;
    int previous_a_col = ( proccess_column + 1 ) % iterations;
    int previous_b_row = ( world_rank + 1 ) % iterations;
    //TOP proccess from this
    int next_proccess_a = proccess_row*iterations + next_a_col;
    //BOTTOM proccess from this
    int previous_proccess_a = proccess_row*iterations + previous_a_col;
    //LEFT proccess from this
    int next_proccess_b = next_b_row*iterations + proccess_column;
    //RIGHT proccess from this
    int previous_proccess_b = previous_b_row*iterations + proccess_column;

    //DO SHIFTING OF CANNON
    for( int i = 0; i < iterations; i++ )
    {
        if( i != 0 )
        {
            //RECIEVE SHIFT BLOCK
            receive_matrix_block(sub_matrix_a, block_size, previous_proccess_a);
            receive_matrix_block(sub_matrix_b, block_size, previous_proccess_b);
        }

        //DO CALCULATION
        matrix_mult(sub_matrix_a, sub_matrix_b, block_size, mulitplication);
        //SUM TO LOCAL C
        sum_matrix_on_place(sub_matrix_c, mulitplication);
        
        reset_matrix(mulitplication);

        //SEND SHIFT BLOCK
        send_matrix(sub_matrix_a, block_size, next_proccess_a);
        send_matrix(sub_matrix_b, block_size, next_proccess_b);
    }
    delete_matrix(mulitplication);
    delete_matrix(sub_matrix_a);
    delete_matrix(sub_matrix_b);

    if( world_rank == 0 )
    {
        //REBUILD C MATRIX
        int n = block_size * iterations;
        int** c = create_empty_matrix(n);
        //COPY LOCAL PART
        copy_matrix_part(sub_matrix_c, 0, 0, 0, 0, n_block, c);
        delete_matrix(sub_matrix_c);
        int row, column;
        for( int i = 1; i < iterations; i++ )
        {
            //RECIEVE OTHER PROCCESS PART
            receive_matrix_block(sub_matrix_c, i);
            row = i / iterations;
            column = i % iterations;
            //COPY PART
            copy_matrix_part(sub_matrix_c, 0, 0, row*block_size, column*block_size, n_block, c);
        }
        //TODO SHOW RESULT
        delete_matrix(c);
    } else
    {
        //SEND LOCAL C CALCULATED
        send_matrix(sub_matrix_c, block_size, 0);
        delete_matrix(sub_matrix_c);
    }
    
    // Finalizar MPI
    MPI_Finalize();
}