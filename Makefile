Mult: 
	gcc mult.c matrix.c -g -Wall -Werror=return-type -pedantic -o mult
Matrix_mult:
	mpicc matrix_mult.c -o matrix_mult
Random_matrix:
	gcc random_matrix.c matrix.c -o random_matrix
Parse_matrix:
	gcc matrix.c parse_matrix.c -o parse_matrix
Summa:
	mpicc SUMMA/summa.c matrix.c -g -Wall -Werror=return-type -pedantic -lm -o summa
