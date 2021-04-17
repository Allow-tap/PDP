#define PI 3.14159265358979323846
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv) {
	if (3 != argc) {
		printf("Usage: stencil num_values num_steps\n");
		return 0;
	}
    int num_values = atoi(argv[1]);
	int num_steps = atoi(argv[2]);
    //Variables for MPI size is the number of the processors and rank is the id
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Status status;
	// Generate values for stencil operation
	double *input=(double*)malloc(num_values*sizeof(double));
    // Allocate data for result

    double *output =(double*)malloc(num_values*sizeof(double));
	//double *tmp =(double*)malloc(size*sizeof(double));
    double h = 2.0*PI/num_values;

	// Divide the number of values into the PEs, excluding PROC 0.
	int chunk = num_values / (size-1);

	int tags[size-1];

	for (int i=0; i < (size-1); i++){
    	tags[i] = 111*(i+1);
  	}

	if (rank == 0){
 	   	for (int i=0; i<num_values; i++){
			input[i]=sin(h*i);
		}

		int tag_idx = 0;
		int istart = tag_idx*chunk;
		for (int i=0; i<num_values; i++) {
			printf("[PROC %d] f(x%d) = %lf\n", rank, i, input[i]);
			if ((i%chunk) == 0) {
				//printf("istart VALUE:%d\n",istart);
				//printf("input+istart POINTER:%p\n",input+istart);
				//printf("input+istart VALUE:%lf\n",*(input+istart));
				printf("[PROC %d] tag_idx = %d, sending to PROC %d\n", rank, tag_idx, tag_idx+1);
				for (int kappa = istart; kappa < (istart+chunk); kappa++){
					printf("[PROC %d] Value at index %d to send: %lf\n", rank, kappa, input[kappa]);
				}
				MPI_Send(input+istart, chunk, MPI_DOUBLE, tag_idx+1, tags[tag_idx], MPI_COMM_WORLD);
				tag_idx++;
				istart = tag_idx*chunk;
				//printf("Rank %d got tag:%d\n",rank, tag_idx);
				//printf("istart SIZE:%ld\n", sizeof(input+istart)/sizeof((input+istart)[0]));
				//printf("input+istart value:%f\n", *(input+istart) );
			}
		}
	} else {
		double *receiver = (double*)malloc(chunk*sizeof(double));
		printf("[PROC %d] tag_idx = %d, receiving from PROC 0\n", rank, rank-1);
		MPI_Recv(receiver, chunk, MPI_DOUBLE, 0, tags[rank-1], MPI_COMM_WORLD, &status);

		for (int kappa = 0; kappa < chunk; kappa++){
			printf("[PROC %d] Receiver value %d is %f:\n", rank, kappa, receiver[kappa]);
		}
    }

	// Stencil values
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH/2;
	const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};

	// Start timer
	double start = MPI_Wtime();

	// Repeatedly apply stencil
	for (int s=0; s<num_steps; s++) {

        // Apply stencil on left boundary with periodic cond
		for (int i=0; i<EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = (i - EXTENT + j + num_values) % num_values;
				result += STENCIL[j] * input[index];
			}
			output[i] = result;
		}

        // Apply stencil on inner points
		for (int i=EXTENT; i<num_values-EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = i - EXTENT + j;
				result += STENCIL[j] * input[index];
			}
			output[i] = result;
		}

        // Apply stencil on right boundary with periodic cond
		for (int i=num_values-EXTENT; i<num_values; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = (i - EXTENT + j) % num_values;
				result += STENCIL[j] * input[index];
			}
			output[i] = result;
		}

		// Swap input and output
		if (s < num_steps-1) {
			double *tmp = input;
			input = output;
			output = tmp;
		}
	}

	// Stop timer
	double my_execution_time = MPI_Wtime() - start;
    //printf("%f\n", my_execution_time);

    /* Write to file
     FILE *file=fopen("output.txt","w");
     for (int i = 0; i < num_values; i++)
         fprintf(file, "%.4f \n", output[i]);
     fclose(file);
     */

	// Clean up
    free(input);
	free(output);
    MPI_Finalize();
	return 0;
}
