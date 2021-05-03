#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdarg.h>
#include <math.h>


void* g_calloc( size_t num, size_t size);
double* generateMatrix( int rows, int cols );


int main(int argc, char **argv) {
    int warn;
    int N = atoi(argv[1]);
    char *output = argv[2];
    double *mat = generateMatrix(N,N);
    FILE *output = NULL;
    output = fopen(output, "w+");

		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fprintf(output, "%lf ", mat[i*N+j]);
				//printf("%lf\n",C[i*N+j]);
			}	
			fprintf(output, "\n");     
}
    fclose(output);
}

void* g_calloc( size_t num, size_t size)
{
    void* p = calloc( num, size );
    if( p == NULL )
        //exitError( "(calloc:%d): Now you've done it. Out of memory!", size );

    return( p );
} // g_calloc


double* generateMatrix( int rows, int cols )
{
    double* numbers = (double*)g_calloc( rows*cols, sizeof(double) );
    for( int i = 0; i < rows; ++i ) 
        for( int j = 0; j < cols; ++j ) 
        {
            const double rnd = rand() * 1.0 / RAND_MAX;
            numbers[i*cols + j] = rnd;
        }
    
    return( numbers );
} // generateMatrix