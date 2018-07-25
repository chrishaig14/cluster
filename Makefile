Matrix_mult:
	mpicc matrix_mult.c -o matrix_mult
Random_matrix:
	gcc random_matrix.c -o random_matrix
Parse_matrix:
	gcc matrix.c parse_matrix.c -o parse_matrix
Summa:
	mpicc SUMMA/summa.c matrix.c -lm -o summa
