#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "utils.h"

double *randMat(const int N) {
    double* Matrix = (double *) malloc(N*N*sizeof(double));

    time_t t;
    srand((unsigned) time(&t));
    int i,j;
    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            Matrix[N*i+j] = rand()%10;
        }
    }

    return Matrix;
}

void printMatrix(double* restrict Matrix, const int N) {
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            printf("%10.6lf ", Matrix[N*i+j]);
        }
        putchar('\n');
    }
}

void writeMatrix(double* restrict Matrix, const int N, const char* fileName)
{
    FILE* fp = fopen(fileName, 'w+');
    if (fp == NULL)
        return;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            fprintf(fp, "%10.6lf ", Matrix[N*i+j]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}