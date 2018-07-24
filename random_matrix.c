#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char const* argv[]) {
    
    if (argc != 3) {
        printf("Usage: random_matrix n output");
    }

    srand(time(NULL));

    int n = atoi(argv[1]);

    char const* output = argv[2];

    FILE* file = fopen(output, "w");
    
    

    char output2[256];
    strcpy(output2, output);
    strcat(output2, ".bin");
    FILE* file2 = fopen(output2, "wb");

    int m = n; // for now, matrices are square...

    fwrite(&m, sizeof(m), 1, file2);
    fwrite(&n, sizeof(n), 1, file2);

    fprintf(file, "%i\n", m);
    fprintf(file, "%i\n", n);

    int num;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            num = rand() % 1000000;
            fprintf(file, "%i\t", num);
            fwrite(&num, sizeof(num), 1, file2);
            printf("%i\t", num);
        }
        printf("\n");
        fprintf(file, "\n");
    }

    fclose(file);
    fclose(file2);

    return 0;
}
