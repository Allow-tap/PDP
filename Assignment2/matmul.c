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

	double *localA,*localB_r,*localB_s,*localC;
	FILE *input = NULL;
	int warn,N, rank, size;        
	double *A, *B, *BC, *C;
	MPI_Status status;
	MPI_Datatype gcoltemp,gcol,lrowtemp,lrow;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	/* Get matrix size */
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
	int chunk = N / size; /* Divide the number of values into PROC */
	
	/* ROOT, store the data into arrays*/
	if (rank == 0 ){
		A = (double *) malloc((N*N)*sizeof(double));
		B = (double *) malloc((N*N)*sizeof(double)); 
		C = (double *) malloc((N*N)*sizeof(double));
	
		/*Iterate the input file and store the elements in arrays A, B */
		for (int i = 0;i < N; i++)
			for (int j = 0; j < N; j++)
				warn=fscanf(input, "%lf", &A[i*N+j]);
				
		for (int i = 0;i < N; i++)
			for (int j = 0; j < N; j++)
				warn=fscanf(input, "%lf", &B[i*N+j]);
		
		/* Create new global datatype for root 0, Coltemp represents 1 col in global B and the stride is N  */
		MPI_Type_vector(N, 1, N, MPI_DOUBLE, &gcoltemp); /* This is 1 column */
  		MPI_Type_commit(&gcoltemp);
  		MPI_Type_create_resized(gcoltemp, 0, 1*sizeof(double), &gcol); /* This is to specify the position of the next column */
  		MPI_Type_commit(&gcol);
	}

	/* Create new local datatype for all processes, the rowtemp represents 1 row in local B and the stride is chunk. */
	MPI_Type_vector(N, 1, chunk, MPI_DOUBLE, &lrowtemp); /* This is 1 row. */
  	MPI_Type_commit(&lrowtemp);
  	MPI_Type_create_resized(lrowtemp, 0, 1*sizeof(double), &lrow); /* Specify the position of the next column */
  	MPI_Type_commit(&lrow);
		
	localC = (double *) malloc((chunk*N)*sizeof(double));
	localA = (double *) malloc((chunk*N)*sizeof(double));
	localB_s = (double *) malloc((N*chunk)*sizeof(double));
	localB_r = (double *) malloc((N*chunk)*sizeof(double));
	
	
	MPI_Barrier(MPI_COMM_WORLD); /* Wait for all PROCS */
	double start = MPI_Wtime();
	MPI_Scatter(A ,chunk*N,MPI_DOUBLE,localA,chunk*N,MPI_DOUBLE,0,MPI_COMM_WORLD); /* Deliver the data */
	MPI_Scatter(B ,chunk,gcol,localB_r,chunk,lrow,0,MPI_COMM_WORLD);	
	
	/* Current rank process send data to rank+1 process; receive data from rank-1 process */
	int SendTo = (rank + 1) % size;
	int RecvFrom = (rank - 1 + size) % size;
	double multStart = MPI_Wtime();
	for (int circle = 0; circle < size; circle++){
		/* One rank's localC has several blocks, determine which block we are computing */
		int indexblock = (rank-circle+size)%size;

		/* Matrix  Multiplication */
		for (int i=0;i<chunk;i++){
			for (int j=0;j<chunk;j++){
				double tempc = 0;
				for (int k=0;k<N;k++){
					tempc += localA[i*N+k] * localB_r[k*chunk+j];
					
				}
				localC[i*N+(indexblock*chunk+j)]=tempc;
			}
		}
		
		/* Copy, send and receive localB */
		for (int i=0;i<chunk*N;i++)
			localB_s[i] = localB_r[i];
				 
		MPI_Sendrecv(localB_s, chunk, lrow, SendTo, circle, localB_r, chunk, lrow, RecvFrom, circle, MPI_COMM_WORLD, &status);
	}
	
	double max_mult_time;
	double multTime = MPI_Wtime() - multStart;
	MPI_Reduce(&multTime, &max_mult_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Gather(localC,N*chunk,MPI_DOUBLE,C,N*chunk,MPI_DOUBLE,0,MPI_COMM_WORLD); /* After all calculation complete, gather */
	MPI_Barrier(MPI_COMM_WORLD);
	double max_runtime;
	double executionTime = MPI_Wtime() - start;
	MPI_Reduce(&executionTime, &max_runtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD); 
	/*Measure all parallel runtime*/
	if (rank == 0){ 
		printf("%f\n", max_runtime); 
		printf("%f\n", max_mult_time);
	}
	////////////////////////////// Print output ///////////////////////////////////////
	if (rank == 0){
		FILE *output = NULL;
		if (NULL == (output = fopen(output_name, "w+"))) {
			perror("Error on opening input file");
			return -2;
		}
		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fprintf(output, "%lf ", C[i*N+j]);
			}	
			fprintf(output, "\n"); 
			
		}
		free(A);
		free(B);
		free(C);
		fclose(input);
		fclose(output);
		MPI_Type_free(&gcoltemp);
		MPI_Type_free(&gcol);
	}
	
	free(localA);
	free(localC);
	free(localB_s);
	free(localB_r);
	MPI_Type_free(&lrowtemp);
	MPI_Type_free(&lrow);
	MPI_Finalize();
  	return 0;
}