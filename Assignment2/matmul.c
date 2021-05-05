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

	//DO MPI STUFF HERE
	double *localA,*localB_r,*localB_s,*localC;
	FILE *input = NULL;
	int warn,N;        
	double *A, *B, *BC, *C;
	int rank, size;
	MPI_Status status;
	MPI_Datatype gcoltemp,gcol;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	// Get matrix size 
	if (rank==0){ 
		if (NULL == (input = fopen(input_name, "r"))) {
			perror("Error on opening input file");
			return -2;
		}
		if (EOF == fscanf(input, "%d", &N)) {
			perror("Error when reading matrix size from input file");
			return -2;
		}
		/*fprintf(stdout, "Matrix size: %d\n", N); // write to screen  */
		//printf("Matrix size: %d * %d\n", N, N); // write to screen  
	}
	MPI_Bcast(&N,1,MPI_INT,0,MPI_COMM_WORLD);
	
	// Divide the number of values into the PEs.
	int chunk = N / size; 
	//printf("\nMy rank is %d, N is %d, chunk is %d\n",rank,N,chunk);
  	
	// rank 0: store the data into arrays
	if (rank == 0 ){

		A = (double *) malloc((N*N)*sizeof(double));
		//NOTE: B stored row first and BC stored column first
		B = (double *) malloc((N*N)*sizeof(double));
		//BC = (double *) malloc((N*N)*sizeof(double));
		C = (double *) malloc((N*N)*sizeof(double));
	
		//Iterate the input file and store the elements in arrays A, B
		//printf("inputA\n");
		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fscanf(input, "%lf", &A[i*N+j]);
				//printf("%lf\n",A[i*N+j]);
			}	
			
		}
		//printf("The last number of A:%lf \n",A[N*N-1]);
		//printf("inputB\n");
		for (int i = 0;i < N; i++){
			for (int j = 0; j < N; j++){
				warn=fscanf(input, "%lf", &B[i*N+j]);
				//BC[j*N+i] = B[i*N+j];
				//printf("%lf\n",B[i*N+j]);
			}	
				
		}
		//printf("The last number of B:%lf \n",B[N*N-1]);
		
		//create new global datatype for root 0
		//the coltemp represents 1 col in global B and the stride is N.
		MPI_Type_vector(N, 1, N, MPI_DOUBLE, &gcoltemp);//This is 1 column.
  		MPI_Type_commit(&gcoltemp);
  		MPI_Type_create_resized(gcoltemp, 0, 1*sizeof(double), &gcol);//This is to specify the position of the next column
  		MPI_Type_commit(&gcol);
	}
	
		
	localC = (double *) malloc((chunk*N)*sizeof(double));
	localA = (double *) malloc((chunk*N)*sizeof(double));
	
	localB_s = (double *) malloc((N*chunk)*sizeof(double));
	localB_r = (double *) malloc((N*chunk)*sizeof(double));
	
	//printf("\nBefore Scatter!\n");
	// Deliver the data
	MPI_Barrier(MPI_COMM_WORLD);
	double start = MPI_Wtime();
	MPI_Scatter(A ,chunk*N,MPI_DOUBLE,localA,chunk*N,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Scatter(B ,chunk,gcol,localB_r,chunk*N,MPI_DOUBLE,0,MPI_COMM_WORLD);	
	//printf("\nAfter Scatter!\n");
	
	/*
	if (rank==2){
		for (int i = 0;i<N;i++){
			for (int j=0;j<chunk;j++){
				printf("%f ",localB_r[chunk*i+j]);
			}
			printf("\n");
		}
	}
	
	if (rank==2){
		for (int i=0;i<N*chunk;i++){
			printf("%f\n",localB_r[i]);	
		}
	}	
	*/
	// Current rank process send data to rank+1 process; receive data from rank-1 process 
	int SendTo = (rank + 1) % size;
	int RecvFrom = (rank - 1 + size) % size;
	/*
	if (rank==3){
		printf("sendto:%d recvfrom:%d\n",SendTo,RecvFrom);	
	}*/
	double spongebob = MPI_Wtime();
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
	double PATRICK; // 
	double mr_crabs = MPI_Wtime() - spongebob; //THE BURGER FLIPPER
	MPI_Reduce(&mr_crabs, &PATRICK, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	//printf("\nAfter circle!\n");
	
	//After all calculation complete, gather
	MPI_Gather(localC,N*chunk,MPI_DOUBLE,C,N*chunk,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	// Stop timer
	double max_runtime;
	double executionTime = MPI_Wtime() - start;
	MPI_Reduce(&executionTime, &max_runtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	//Measure all parallel runtime
	if (rank == 0){ printf("%f\n", max_runtime); 
					printf("%f\n", PATRICK);
					}

	/////////////////////////////////////////////////////////////////////////////
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
		MPI_Type_free(&gcoltemp);
		MPI_Type_free(&gcol);
		//printf("\nAfter write!\n");	
	}
	
	free(localA);
	free(localC);
	free(localB_s);
	free(localB_r);
	
	MPI_Finalize();
	
  	return 0;
}


