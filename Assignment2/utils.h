#ifndef MATRIX_H
#define MATRIX_H

double *randMat(const int N);
void printMatrix(double* restrict Matrix, const int N);
void writeMatrix(double* restrict Matrix, const int N, const char* fileName);

#endif