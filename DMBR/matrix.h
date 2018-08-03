#ifndef __MATRIX_H__
#define __MATRIX_H__

void validate_create_matrix(int*);
int* create_empty_matrix(int,int);
int* create_random_matrix(int,int,int);
int* transpose_matrix(int*,int,int,int);
void delete_matrix(int*);
int* calculate_matrix_C(int*,int ,int ,int );
void read_rows_cols(const char*,int*,int*);
int* parse_matrix(const char*,int);
void write_matrix(int*,int,int,const char*);

#endif