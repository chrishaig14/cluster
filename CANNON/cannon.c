#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../matrix.h"

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
    for (size_t i = 0; i < n; i++) {
        MPI_Recv(dest[i], n, MPI_INT, from, MATRIX_BLOCK, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
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
            matrix[i][j] = 0;
        }
    }
}

void copy_matrix_part_to(int** a, 
    int start_row, int start_col, 
    int dest_row, int dest_col, 
    int n_copy,
    int** dest)
{
    for( int i = 0; i < n_copy; i++)
    {
        for( int j = 0; j < n_copy; j++ )
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
    copy_matrix_part_to(a, start_row, start_col, 0, 0, n_copy, dest);
}

int mod(int a, int b)
{
    int ret = a % b;
    if(ret < 0)
    {
        ret+=b;
    }
    return ret;
}

void intialize_cannon_blocks(int amount_blocks, int iterations, int* n_block, 
            int** a, int** b, int** a_proccess, int** b_proccess )
{
    //INITIALIZE PROCCES 0
    copy_matrix_part(a, 0, 0, *n_block, a_proccess);
    copy_matrix_part(b, 0, 0, *n_block, b_proccess);
    
    int** sub_a = create_empty_matrix(*n_block);
    int** sub_b = create_empty_matrix(*n_block);

    int block_row, block_column, shift_row, shift_col;
    int start_col_a, start_row_a, start_col_b, start_row_b;
    //INITIALIZE OTHER PROCCESS
    for( int i = 1; i < amount_blocks; i++) 
    {
        //GET ROw,COLUMN of grid
        block_row = i / iterations;
        block_column = i % iterations;
        //CALCULATE SHIFTS
        shift_row = mod( block_row + block_column, iterations);
        shift_col = mod( block_column +  block_row, iterations);
        //CALCULATE indexes from which to copy matrix block
        start_row_a = block_row*(*n_block);
        start_col_a = shift_col*(*n_block);
        start_row_b = shift_row*(*n_block);
        start_col_b = block_column*(*n_block);
        //COPY part of matrixes
        copy_matrix_part(a, start_row_a, start_col_a, *n_block, sub_a);
        copy_matrix_part(b, start_row_b, start_col_b, *n_block, sub_b);
        //SEND block size
        MPI_Send(n_block, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD);
        //SEND blocks of A and B
        send_matrix_block(sub_a, *n_block, i);
        send_matrix_block(sub_b, *n_block, i);
    }
    delete_matrix(sub_a, *n_block);
    delete_matrix(sub_b, *n_block);
}

int main(int argc, char** argv) 
{
    if( argc != 4 )
    {
        printf("Usage: cannon matrix_a matrix_b result\n");
        return 0;
    }
    // Inicializar MPI
    MPI_Init(NULL, NULL);

    // Obtener el numero de procesos
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Obtener el rank (id) del proceso actual
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    //TODO SEARCH MINIMUM SQUARE TO USE PROCCESS
    double root = sqrt((double)world_size);
    double floor_root = floor(root);
    if( root - floor_root != 0 )
    {
        printf("Can't run Cannon algorithms with a number of processes that does not have a square root\n");
        return 1;
    }

    int iterations = (int)root;
    int block_size; 
    int** sub_matrix_a; 
    int** sub_matrix_b; 
    int** sub_matrix_c;
    if( world_rank == 0 )
    {
        int n, n_a, m_a, n_b, m_b;
        int** a = parse_matrix(argv[1], &m_a, &n_a);
        int** b = parse_matrix(argv[2], &m_b, &n_b);
        if( n_a != m_a || m_a != n_b || n_b != m_b || n_a % iterations != 0 )
        {
            block_size = -1;
            if( n_a % iterations != 0 )
            {
                printf("Can't run Cannon because matrixes cant be divided in blocks of order %i .\n", iterations);
            } else
            {
                printf("Can't run Cannon because matrixes are not squares a:%ix%i b:%ix%i.\n",m_a,n_a,m_b,n_b);
            }
            
            //TODO CLOSE FILES
            for( int i = 1; i < world_rank; i++) 
            {
                //SEND DO NOT RESOLVE
                MPI_Send((void*)&block_size, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD); 
            }
            free2d(a);
            free2d(b);
            return 1;
        }
        n = n_a;      

        if( iterations > n )
        {
            printf("Resolving multiplication localy because the order of the proccess is greatear than th order of the matrix \n");
            int** c = create_empty_matrix(n);
            block_size = -1;
            
            for( int i = 1; i < world_rank; i++) 
            {
                //SEND DO NOT RESOLVE TO OTHERS
                MPI_Send((void*)&block_size, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD); 
            }
            //RESOLVE LOCALLY
            matrix_mult(a, b, c, n, n, n);
            
            free2d(a);
            free2d(b);
            delete_matrix(c,n);
            return 0;
        } 
        //CALCULATE BLOCK SIZE
        printf("Matrix A %ix%i:\n", m_a, n_a);
        print_matrix(a, m_a, n_a);
        printf("Matrix B %ix%i:\n", m_b, n_b);
        print_matrix(b, m_a, n_a);
        block_size = n / iterations;
        
        //START BLOCK DISTRIBUTION
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        intialize_cannon_blocks(world_size, iterations, &block_size, a, b, sub_matrix_a, sub_matrix_b);
        free2d(a);
        free2d(b);
    } else 
    {
        //RECIEVE INITIAL DATA FOR CANNON
        MPI_Recv(&block_size, 1, MPI_INT, 0, BLOCK_SIZE, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        //ERROR OCCURED or RESOLVED LOCALLY
        if( block_size <= 0 )
        {
            printf("P:%i, Not resolving matrix in this procces \n",world_rank);
            return 0;
        }
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        receive_matrix_block(block_size, 0, sub_matrix_a);
        receive_matrix_block(block_size, 0, sub_matrix_b);
    }

    sub_matrix_c = create_empty_matrix(block_size);

    int** multiplication = create_empty_matrix(block_size);
    //CALCULATA ROW,COL of proccess in GRID
    int proccess_row = world_rank / iterations;
    int proccess_column = world_rank % iterations;
    //CALCULATE NEIGHBOUR ROW,COL of proccess in GRID
    int next_a_col = mod( proccess_column - 1, iterations);
    int next_b_row = mod( proccess_row - 1, iterations);
    int previous_a_col = mod( proccess_column + 1, iterations);
    int previous_b_row = mod( proccess_row + 1, iterations);
    //CALCULATE NEIGHBOUR PROCCESS NUMBER
    int next_proccess_a = proccess_row*iterations + next_a_col;
    int previous_proccess_a = proccess_row*iterations + previous_a_col;
    int next_proccess_b = next_b_row*iterations + proccess_column;
    int previous_proccess_b = previous_b_row*iterations + proccess_column;
    
    //DO MUTLIPLY AND PASS BLOCK
    for( int i = 0; i < iterations; i++ )
    {
        if( i != 0 )
        {
            //RECIEVE BLOCK
            receive_matrix_block(block_size, previous_proccess_a, sub_matrix_a);
            receive_matrix_block(block_size, previous_proccess_b, sub_matrix_b);
        }

        //DO CALCULATION
        matrix_mult(sub_matrix_a, sub_matrix_b, multiplication, block_size, block_size, block_size);
        //SUM TO LOCAL C
        sum_matrix_on_place(sub_matrix_c, multiplication, block_size);
        
        reset_matrix(multiplication, block_size);

        //SEND BLOCK
        if( i != iterations - 1 )
        {
            send_matrix_block(sub_matrix_a, block_size, next_proccess_a);
            send_matrix_block(sub_matrix_b, block_size, next_proccess_b);
        }
    }
    delete_matrix(multiplication, block_size);
    delete_matrix(sub_matrix_a, block_size);
    delete_matrix(sub_matrix_b, block_size);

    if( world_rank == 0 )
    {
        
        int n = block_size * iterations;
        int** c = create_empty_matrix(n);
        //COPY LOCAL PART
        copy_matrix_part_to(sub_matrix_c, 0, 0, 0, 0, block_size, c);
        
        int row, column;
        for( int i = 1; i < world_size; i++ )
        {
            //RECIEVE OTHER PROCCESS C PART
            receive_matrix_block(block_size, i, sub_matrix_c);
            
            row = i / iterations;
            column = i % iterations;
            //COPY PART TO RESULT MATRIX
            copy_matrix_part_to(sub_matrix_c, 0, 0, row*block_size, column*block_size, block_size, c);
        }
        delete_matrix(sub_matrix_c, block_size);
        //SHOW RESULT
        write_matrix(c, n, n, argv[3]);
        printf("Matrix C %ix%i:\n", n, n);
        print_matrix(c, n, n);
        delete_matrix(c, n);
    } else
    {
        //SEND LOCAL C CALCULATED
        send_matrix_block(sub_matrix_c, block_size, 0);
        delete_matrix(sub_matrix_c, block_size);
    }
    
    // Finalizar MPI
    MPI_Finalize();
}