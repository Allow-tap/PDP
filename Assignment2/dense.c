#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0

int main(int argc, char **argv) {

	char *input_name = argv[1];
	char *output_name = argv[2];

	// Get matrix size  
	int N; 
	FILE *input = NULL;
	if (NULL == (input = fopen(input_name, "r"))) {
		perror("Error on opening input file");
		return -2;
	}
	if (EOF == fscanf(input, "%d", &N)) {
		perror("Error when reading matrix size from input file");
		return -2;
	}
	/*fprintf(stdout, "Matrix size: %d\n", N); // write to screen  */
	printf("Matrix size: %d * %d\n", N, N); // write to screen  


	//DO MPI STUFF HERE
	int rank, size;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	// we have the data split it equally among PROCS THEN do the calculations inside each proc, then join the results
	
	// Divide the number of values into the PEs.
	int warn;        
	double *A, *B, *C;
	int chunk = N / size; 
	double *rcv = (double *)malloc(chunk*N*sizeof(double));

	if (rank == 0 ){

	A = (double *) malloc((N*N)*sizeof(double));
	B = (double *) malloc((N*N)*sizeof(double));
	C = (double *) malloc((N*N)*sizeof(double));
	
	//Iterate the input file and store the elements in arrays A, B
	printf("inputA\n");
	for (int i = 0;i < N; i++){
		for (int j = 0; j < N; j++){
			warn=fscanf(input, "%lf", &A[i*N+j]);
			//printf("%lf\n",A[i*N+j]);
		}	
			
	}
	printf("The last number of A:%lf\n",A[N*N-1]);
	printf("inputB\n");
	for (int i = 0;i < N; i++){
		for (int j = 0; j < N; j++){
			warn=fscanf(input, "%lf", &B[i*N+j]);
			//printf("%lf\n",B[i*N+j]);
		}	
			
	}
	printf("The last number of B:%lf\n",B[N*N-1]);

	MPI_Scatter(input+rank*chunk, chunk*rank, MPI_DOUBLE, rcv, chunk*rank, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	//MULTIPLY////////////////////////////////////////////////////////////////
    double factor;
    for (int i=0; i<N; i++) {
        for (int k=0; k<N; k++) {
            factor = A[i+N*k];
            for (int j=0; j<N; j++) {
                C[N*i+j] += B[k+N*j]*factor;
            }
        }
    }
	/////////////////////////////////////////////////////////////////////////////

	}
	


	/// Print output
	FILE *output = NULL;
	if (NULL == (output = fopen(output_name, "w+"))) {
		perror("Error on opening input file");
		return -2;
	}
	for (int i = 0;i < N; i++){
		for (int j = 0; j < N; j++){
			warn=fprintf(output, "%lf ", C[i*N+j]);
			//printf("%lf\n",C[i*N+j]);
		}	
		fprintf(output, "\n"); 
			
	}
	free(A);
	free(B);
	free(C);
	fclose(input);
	fclose(output);
	MPI_Finalize();
  	return 0;
}



