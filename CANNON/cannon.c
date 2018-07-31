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
            a[i][j] = rand() % 10;//1000000;
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

void print_matrix(int** matrix, int n)
{
    for( int i = 0; i < n; i++ )
    {
        for( int j = 0; j < n; j++ )
        {
            printf(" %i ", (int)matrix[i][j]);
        }
        printf("\n");
    }
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
    copy_matrix_part(a, 0, 0, *n_block, a_proccess);
    copy_matrix_part(b, 0, 0, *n_block, b_proccess);
    
    int** sub_a = create_empty_matrix(*n_block);
    int** sub_b = create_empty_matrix(*n_block);

    int block_row, block_column, shift_row, shift_col;
    int start_col_a, start_row_a, start_col_b, start_row_b;
    for( int i = 1; i < amount_blocks; i++) 
    {
        //printf("Initializing block %i\n", i);
        block_row = i / iterations;
        block_column = i % iterations;
        //printf("Block index %i,%i\n", block_row,block_column);
        shift_row = mod( block_row + block_column, iterations);
        shift_col = mod( block_column +  block_row, iterations);
        
        start_row_a = block_row*(*n_block);
        start_col_a = shift_col*(*n_block);
        start_row_b = shift_row*(*n_block);
        start_col_b = block_column*(*n_block);
        //printf("Copying a part from %i,%i\n", start_row_a, start_col_a);
        copy_matrix_part(a, start_row_a, start_col_a, *n_block, sub_a);
        //printf("Copying b part from %i,%i\n", start_row_b, start_col_b);
        copy_matrix_part(b, start_row_b, start_col_b, *n_block, sub_b);
        
        
        MPI_Send(n_block, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD);
        //printf("Sent to proccess %i block size %i\n", i, *n_block);

        send_matrix_block(sub_a, *n_block, i);
        //printf("Sending to proccess %i block  b\n", i);
        //print_matrix(sub_b, *n_block);

        send_matrix_block(sub_b, *n_block, i);
        //printf("Sent to proccess %i block  a\n", i);
        //print_matrix(sub_a,*n_block);
    }
    delete_matrix(sub_a, *n_block);
    delete_matrix(sub_b, *n_block);
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
    double root = sqrt((double)world_size);
    double floor_root = floor(root);
    if( root - floor_root != 0 )
    {
        printf("Can't run Cannon algorithms with a number of processes that does not have a square root\n");
        return 1;
    }

    //printf("Running in %i proccess \n", world_rank);
    int iterations = (int)root;
    int block_size; 
    int** sub_matrix_a; 
    int** sub_matrix_b; 
    int** sub_matrix_c;
    if( world_rank == 0 )
    {
        int n = rand() % 4999 + 1;
        //printf("Iterations %i\n", iterations);
        //printf("ORIGINAL N is %i\n", n);
        n += iterations - ( n % iterations );
        int** a = create_random_matrix(n);
        //printf("MATRIX A\n");
        //print_matrix(a, n);
        int** b = create_random_matrix(n);
        
        //printf("MATRIX B\n");
        //print_matrix(b, n);
        
        if( iterations > n )
        {
            int** c = create_empty_matrix(n);
            block_size = -1;
            //RESOLVE LOCALLY
            for( int i = 1; i < world_rank; i++) 
            {
               MPI_Send((void*)&block_size, 1, MPI_INT, i, BLOCK_SIZE, MPI_COMM_WORLD); 
            }
            //SEND DO NOT RESOLVE
            matrix_mult(a,b,n,c);
            //printf("MATRIX C\n");
            print_matrix(c, n);
            delete_matrix(a,n);
            delete_matrix(b,n);
            delete_matrix(c,n);
            return 0;
        } 
        //Makes it possible to always calculate the block size
        
        //printf("MATRIX SIZE = %i\n", n);
        block_size = n / iterations;
        //printf("BLOCK SIZE = %i\n", block_size);
        //START multiplying matrixes
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        intialize_cannon_blocks(world_size, iterations, &block_size, a, b, sub_matrix_a, sub_matrix_b);
        delete_matrix(a, n);
        delete_matrix(b, n);
    } else 
    {
        //RECIEVE INITIAL DATA FOR CANNON
        //printf("P:%i, Waiting to recieve initial size of block\n",world_rank);
        MPI_Recv(&block_size, 1, MPI_INT, 0, BLOCK_SIZE, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        //printf("P:%i, Recieved %i\n",world_rank, block_size);
        if( block_size <= 0 )
        {
            printf("P:%i, was resolved locally\n",world_rank);
            return 0;
        }
        sub_matrix_a = create_empty_matrix(block_size);
        sub_matrix_b = create_empty_matrix(block_size);
        receive_matrix_block(block_size, 0, sub_matrix_a);
        receive_matrix_block(block_size, 0, sub_matrix_b);
    }

    //DO CANNON
    sub_matrix_c = create_empty_matrix(block_size);
    int** multiplication = create_empty_matrix(block_size);
    int proccess_row = world_rank / iterations;
    int proccess_column = world_rank % iterations;
    //printf("P:%i, row: %i, col: %i \n", world_rank, proccess_row, proccess_column);
    int next_a_col = mod( proccess_column - 1, iterations);
    int next_b_row = mod( proccess_row - 1, iterations);
    int previous_a_col = mod( proccess_column + 1, iterations);
    int previous_b_row = mod( proccess_row + 1, iterations);
    //TOP proccess from this
    int next_proccess_a = proccess_row*iterations + next_a_col;
    //BOTTOM proccess from this
    int previous_proccess_a = proccess_row*iterations + previous_a_col;
    //LEFT proccess from this
    int next_proccess_b = next_b_row*iterations + proccess_column;
    //RIGHT proccess from this
    int previous_proccess_b = previous_b_row*iterations + proccess_column;
    /*printf("P:%i, next A:: process: %i, row: %i, col: %i\n", world_rank, next_proccess_a, proccess_row, next_a_col);
    printf("P:%i, next B:: process: %i, row: %i, col: %i\n", world_rank, next_proccess_b, next_b_row, proccess_column);
    printf("P:%i, previous A:: process: %i, row: %i, col: %i\n", world_rank, previous_proccess_a, proccess_row, previous_a_col);
    printf("P:%i, previous B:: process: %i, row: %i, col: %i\n", world_rank, previous_proccess_b, previous_b_row, proccess_column);*/
    //DO SHIFTING OF CANNON
    for( int i = 0; i < iterations; i++ )
    {
        if( i != 0 )
        {
            //RECIEVE SHIFT BLOCK
            //printf("P:%i, receiving matrixes from A:%i, B:%i\n",world_rank, previous_proccess_a, previous_proccess_b);
            receive_matrix_block(block_size, previous_proccess_a, sub_matrix_a);
            receive_matrix_block(block_size, previous_proccess_b, sub_matrix_b);
        }

        //DO CALCULATION
        matrix_mult(sub_matrix_a, sub_matrix_b, block_size, multiplication);
        //SUM TO LOCAL C
        sum_matrix_on_place(sub_matrix_c, multiplication, block_size);
        
        reset_matrix(multiplication, block_size);

        //SEND SHIFT BLOCK
        if( i != iterations - 1 )
        {
            //printf("P:%i, passing matrixes to A:%i, B:%i\n",world_rank, next_proccess_a, next_proccess_b);
            send_matrix_block(sub_matrix_a, block_size, next_proccess_a);
            send_matrix_block(sub_matrix_b, block_size, next_proccess_b);
        }
    }
    delete_matrix(multiplication, block_size);
    delete_matrix(sub_matrix_a, block_size);
    delete_matrix(sub_matrix_b, block_size);

    if( world_rank == 0 )
    {
        //REBUILD C MATRIX
        int n = block_size * iterations;
        int** c = create_empty_matrix(n);
        //COPY LOCAL PART
        copy_matrix_part_to(sub_matrix_c, 0, 0, 0, 0, block_size, c);
        
        int row, column;
        for( int i = 1; i < world_size; i++ )
        {
            //RECIEVE OTHER PROCCESS PART
            //printf("Receiving C block from %i\n", i);
            receive_matrix_block(block_size, i, sub_matrix_c);
            
            row = i / iterations;
            column = i % iterations;
            //COPY PART
            /*printf("Copyin matrix C to index in C %i,%i \n", row*block_size, column*block_size );
            print_matrix(sub_matrix_c, block_size);*/
            copy_matrix_part_to(sub_matrix_c, 0, 0, row*block_size, column*block_size, block_size, c);
        }
        delete_matrix(sub_matrix_c, block_size);
        //TODO SHOW RESULT
        //printf("P:%i, C MATRIX\n",world_rank);
        //print_matrix(c,n);
        delete_matrix(c, n);
    } else
    {
        //SEND LOCAL C CALCULATED
        //printf("P:%i, sending resulting c block\n",world_rank);
        send_matrix_block(sub_matrix_c, block_size, 0);
        delete_matrix(sub_matrix_c, block_size);
    }
    
    // Finalizar MPI
    MPI_Finalize();
}