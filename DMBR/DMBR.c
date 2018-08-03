#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include "DMBR.h"


void distribute_matrix( int* matrix_A,
						int* matrix_B, 
						int rows_A_send, 
						int rows_BT_send,
						int* local_matrix_A,
						int* local_matrix_B) {
	
	MPI_Scatter(matrix_A, rows_A_send, MPI_INT, local_matrix_A, rows_A_send, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(matrix_B, rows_BT_send, MPI_INT,local_matrix_B, rows_BT_send, MPI_INT, 0,MPI_COMM_WORLD);

}


void pp(int* v, int c,int r){
	printf(" %d ", r);
	for (int i = 0; i < c; ++i) {
		printf(" %d ",v[i] );
	}
	printf("\n");
}

int* calculate_product( int* local_matrix_A,
						int* local_matrix_B, 
						int num_proc, 
						int proc_rank, 
						int rows_BT_send, 
						int extra_rows,
						int extra_cols,
						int COLS_A,
						int ROWS_A,
						int COLS_B,
						int ROWS_B
						) {

	int rows_A_by_proc = ((ROWS_A + extra_rows) / num_proc);
	int cols_B_by_proc = ((COLS_B + extra_cols) / num_proc);
	int size_local_c =((ROWS_A + extra_rows) / num_proc) * (COLS_B + extra_cols);
	int source = (proc_rank - 1 + num_proc) % num_proc;
	int dest = (proc_rank + 1) % num_proc;
	int* result = NULL;
	int* local_matrix_C = (int*)malloc(sizeof(int) * size_local_c);
	int *local_matrix_B_Aux = NULL;
	validate_create_matrix(local_matrix_C);


	if (proc_rank % 2 != 0) {
		local_matrix_B_Aux = (int*) malloc(sizeof(int) * rows_BT_send);
	}

	for (int it = 0; it < num_proc; ++it) {
		for (int i = 0; i < rows_A_by_proc; ++i) {
			for (int k = 0; k < cols_B_by_proc; ++k) {
				int index = ((proc_rank - it + num_proc) % num_proc) * cols_B_by_proc + (ROWS_A + extra_rows) * i + k;
				local_matrix_C[index] = 0;
				for (int j = 0; j < COLS_A; ++j) {
					local_matrix_C[index] += local_matrix_A[j+i*COLS_A] * local_matrix_B[j+k*COLS_A];
				}
			}
		}
		if (proc_rank % 2 == 0) {

			MPI_Send(local_matrix_B, rows_BT_send, MPI_INT,dest, 0, MPI_COMM_WORLD);
			MPI_Recv(local_matrix_B, rows_BT_send, MPI_INT,source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		} else {

			int *aux;

			MPI_Recv(local_matrix_B_Aux, rows_BT_send, MPI_INT,source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(local_matrix_B, rows_BT_send, MPI_INT,dest, 0, MPI_COMM_WORLD);

			aux = local_matrix_B;
			local_matrix_B = local_matrix_B_Aux;
			local_matrix_B_Aux = aux;
			
		}
		
	}

	int* matrix_C = NULL;
	if (proc_rank == 0) {
		matrix_C = (int*)malloc(sizeof(int) * (ROWS_A+extra_rows)*(COLS_B+extra_cols));
		validate_create_matrix(matrix_C);
	}

	MPI_Gather(local_matrix_C, size_local_c, MPI_INT,matrix_C, size_local_c, MPI_INT, 0,MPI_COMM_WORLD);

	if (proc_rank == 0){
		result = calculate_matrix_C(matrix_C,ROWS_A,COLS_B,extra_cols);
		delete_matrix(matrix_C);
	}

	if (local_matrix_B_Aux != NULL) {
		delete_matrix(local_matrix_B_Aux);
	}

	delete_matrix(local_matrix_C);

	return result;

}

int validate_product(int cols_A, int rows_b) {
	return cols_A != rows_b;
}

void decomposition_columns(int proc_rank, int num_proc, const char* file_matrix_A, const char* file_matrix_B) {


	int* matrix_A;
	int* matrix_B;
	int* matrix_B_transpose;
	int COLS_A, ROWS_A, COLS_B, ROWS_B;
	int extra_rows, extra_cols; 

	if (proc_rank == 0) {

		read_rows_cols(file_matrix_A,&COLS_A, &ROWS_A);
		read_rows_cols(file_matrix_B, &COLS_B, &ROWS_B);
		extra_rows = (num_proc - (ROWS_A % num_proc)) % num_proc;
		extra_cols = (num_proc - (COLS_B % num_proc)) % num_proc;

		int dimensions[4] = {COLS_A,ROWS_A,COLS_B,ROWS_B};
		MPI_Bcast(&dimensions, 4, MPI_INT, 0, MPI_COMM_WORLD);

		if ( validate_product(COLS_A,ROWS_B) )
			return;

		matrix_A = parse_matrix(file_matrix_A,extra_rows);
		matrix_B = parse_matrix(file_matrix_B,0);

		matrix_B_transpose = transpose_matrix(matrix_B,ROWS_B,COLS_B,extra_cols);


	} else {

		int dimensions[4];
		MPI_Bcast(&dimensions, 4, MPI_INT, 0, MPI_COMM_WORLD);
		COLS_A = dimensions[0];
		ROWS_A = dimensions[1];
		COLS_B = dimensions[2];
		ROWS_B = dimensions[3];

		if ( validate_product(COLS_A,ROWS_B) ) 
			return;

		extra_rows = (num_proc - (ROWS_A % num_proc)) % num_proc;
		extra_cols = (num_proc - (COLS_B % num_proc)) % num_proc;
	}

	int rows_A_send = ((ROWS_A + extra_rows) / num_proc)*COLS_A;
	int rows_BT_send = ((COLS_B + extra_cols) / num_proc)*ROWS_B;

	int *local_matrix_A = (int*) malloc(sizeof(int) * rows_A_send);
	validate_create_matrix(local_matrix_A);
	
	int *local_matrix_B = (int*) malloc(sizeof(int) * rows_BT_send);
	validate_create_matrix(local_matrix_B);

	distribute_matrix(matrix_A,matrix_B_transpose,rows_A_send,rows_BT_send,local_matrix_A,local_matrix_B);

	int* result = calculate_product(local_matrix_A,local_matrix_B,num_proc,proc_rank,rows_BT_send,extra_rows,extra_cols,COLS_A,ROWS_A,COLS_B,ROWS_B);

	if (proc_rank == 0){
		write_matrix(result,ROWS_A,COLS_B,"result_DMBR");
		delete_matrix(result);
		delete_matrix(matrix_A);
		delete_matrix(matrix_B);
		delete_matrix(matrix_B_transpose);
	}

	delete_matrix(local_matrix_A);
	delete_matrix(local_matrix_B);

}