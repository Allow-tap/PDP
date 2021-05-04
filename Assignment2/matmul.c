#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0

double executionTime = 0.0;
double max_parallel_runtime=0.0;

int main(int argc, char **argv) {
	//Make a check that number of elements in divisible by NP
	
	char *input_name = argv[1]; //input
	char *output_name = argv[2]; //output

	double *localA,*localB_r,*localB_s,*localC; //data
	FILE *input = NULL;
	int warn,N,rank,size;    

	double *A, *B, *BC, *C;
	MPI_Status status;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	if (rank==0){ 
		if (NULL == (input = fopen(input_name, "r"))) {
			perror("Error on opening input file");
			return -2;
		}
		if (EOF == fscanf(input, "%d", &N)) {
			perror("Error when reading matrix size from input file");
			return -2;
		}
	}
	MPI_Bcast(&N,1,MPI_INT,0,MPI_COMM_WORLD);

	if ( rank == 0 && (N % size) != 0){
		printf("Number of elements is not divisible by # PROC\n");
		//printf("Learn to divide numbers\n.... c ya\n");
		exit(1);
	} 
	// Divide the number of values into the PROC
	int chunk = N / size; 
	// rank 0: store the data into arrays
	// we have the data split it equally among PROCS THEN do the calculations inside each proc, then join the results
	if (rank == 0 ){

		A = (double *) malloc((N*N)*sizeof(double));
		//NOTE: B stored row first and BC stored column first
		B = (double *) malloc((N*N)*sizeof(double));
		BC = (double *) malloc((N*N)*sizeof(double));
		C = (double *) malloc((N*N)*sizeof(double));
	
		//Iterate the input file and store the elements in arrays A, B
		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fscanf(input, "%lf", &A[i*N+j]);
				//printf("%lf\n",A[i*N+j]);
			}	
		}
		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fscanf(input, "%lf", &B[i*N+j]);
				BC[j*N+i] = B[i*N+j];
			}		
		}
	}
	
	localC = (double *) malloc((chunk*N)*sizeof(double));
	localA = (double *) malloc((chunk*N)*sizeof(double));
	
	localB_s = (double *) malloc((N*chunk)*sizeof(double));
	localB_r = (double *) malloc((N*chunk)*sizeof(double));

	// Deliver the data
	MPI_Barrier(MPI_COMM_WORLD);
	// Start the timer. Do not include file operations.
	executionTime = MPI_Wtime();
	MPI_Scatter(A ,chunk*N,MPI_DOUBLE,localA,chunk*N,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Scatter(BC,chunk*N,MPI_DOUBLE,localB_r,chunk*N,MPI_DOUBLE,0,MPI_COMM_WORLD);	
	
	// Current rank process send data to rank+1 process; receive data from rank-1 process 
	int SendTo = (rank + 1) % size;
	int RecvFrom = (rank - 1 + size) % size;
	
	for (int circle = 0; circle < size; circle++){
		//One rank's localC has several blocks, determine which block we are computing
		int indexblock = (rank-circle+size)%size;
		//Multiplication
		for (int i=0;i<chunk;i++){
			for (int j=0;j<chunk;j++){
				double tempc = 0;
				for (int k=0;k<N;k++){
					tempc=tempc + localA[i*N+k] * localB_r[j*N+k];
				}
				localC[i*N+(indexblock*chunk+j)]=tempc;
			}
		}
		//Copy, send and receive localB
		for (int i=0;i<N*chunk;i++){
			localB_s[i] = localB_r[i];	
		}		 
		MPI_Sendrecv(localB_s, chunk*N, MPI_DOUBLE, SendTo, circle, localB_r, chunk*N, MPI_DOUBLE, RecvFrom, circle, MPI_COMM_WORLD, &status);
		
	}
	//After all calculation complete, gather
	//MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gather(localC,N*chunk,MPI_DOUBLE,C,N*chunk,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	// Measure time. Exclude file write. 
    executionTime = MPI_Wtime() - executionTime;
	/*
	if (rank == 0){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	if (rank == 1){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	if (rank == 2){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	if (rank == 3){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	if (rank == 4){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	if (rank == 5){ printf("PROC[%d]\t%f\n", rank, executionTime);}
	*/
	MPI_Reduce(&executionTime, &max_parallel_runtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	if (rank == 0){ printf("%f\n", max_parallel_runtime);}
	
	if (rank == 0){
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
		free(BC);
		free(C);
		fclose(input);
		fclose(output);
	}
	
	free(localA);
	free(localC);
	free(localB_s);
	free(localB_r);
	
	MPI_Finalize();
	
  	return 0;
}