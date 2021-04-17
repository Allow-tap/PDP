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
	MPI_Request r_recv_request[size-2], r_send_request[size-2], l_recv_request[size-2], l_send_request[size-2];
    double *output =(double*)malloc(num_values*sizeof(double));
	//double *tmp =(double*)malloc(size*sizeof(double));
    double h = 2.0*PI/num_values;

	// Divide the number of values into the PEs, excluding PROC 0.
	int chunk = num_values / (size-1);

	int tags[size-1];
	int sendL_tags[size-2], sendR_tags[size-2];

	for (int i=0; i < (size-1); i++){
    	tags[i] = 111*(i+1);
  	}
	for (int i=0; i < (size-2); i++){
		sendL_tags[i] = 111*(i+1);
		sendR_tags[i] = 111*(i+1);
  	}

	if (rank == 0){
 	   	for (int i=0; i<num_values; i++){
			input[i]=sin(h*i);
		}

		int tag_idx = 0;
		int istart = tag_idx*chunk;
		//Send the chunks to each processor
		for (int i=0; i<num_values; i++) {
			printf("[PROC %d] f(x%d) = %lf\n", rank, i, input[i]);
			if ((i%chunk) == 0) {
				//printf("[PROC %d] tag_idx = %d, sending to PROC %d\n", rank, tag_idx, tag_idx+1);
				for (int kappa = istart; kappa < (istart+chunk); kappa++){
					printf("[PROC %d] Value at index %d to send: %lf\n", rank, kappa, input[kappa]);
				}
				MPI_Send(input+istart, chunk, MPI_DOUBLE, tag_idx+1, tags[tag_idx], MPI_COMM_WORLD);
				tag_idx++;
				istart = tag_idx*chunk;
			}
		}
	} else {
		double *buffer = (double*)malloc(chunk*sizeof(double));
		printf("[PROC %d] tag_idx = %d, receiving from PROC 0\n", rank, rank-1);
		MPI_Recv(buffer, chunk, MPI_DOUBLE, 0, tags[rank-1], MPI_COMM_WORLD, &status);

		//Except PROC[0] every other follows --> PROC[1] will have from the left the 2 last elements of PROC[size-1] and on the right the 2 first elements of PROC[2]
		/************************************--> PROC[2] L(2 el from): PROC[1] R(2 el from): PROC[3]
		 ************************************--> PROC[3] L(2 el from): PROC[2] R(2 el from): PROC[4]*/


		/************************* ***********************************************************************************************************/

		// Stencil values
		const int STENCIL_WIDTH = 5;
		const int EXTENT = STENCIL_WIDTH/2;
		const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};
		double *left = (double*)malloc(EXTENT*sizeof(double));
		double *right = (double*)malloc(EXTENT*sizeof(double));

		if (rank != 1) {
			// Send to left processor (which receives into its right buffer).
			MPI_Isend(buffer, EXTENT, MPI_DOUBLE, rank-1, sendL_tags[rank-1], MPI_COMM_WORLD, &l_send_request[rank-1]);
			// Receive from left processor (into this processor's left buffer).
			MPI_Irecv(left, EXTENT, MPI_DOUBLE, rank-1, sendR_tags[rank-1], MPI_COMM_WORLD,&l_recv_request[rank-1]);
        	MPI_Wait(&l_send_request[rank-1], &status);
        	MPI_Wait(&l_recv_request[rank-1], &status);
		} else {
			// Set the first processor to send to the last
			MPI_Isend(buffer, EXTENT, MPI_DOUBLE, size-1, sendL_tags[size-2], MPI_COMM_WORLD, &l_send_request[size-2]);
			// Set the first processor to receive from the last.
			MPI_Irecv(left, EXTENT, MPI_DOUBLE, size-1, sendR_tags[size-2], MPI_COMM_WORLD,&l_recv_request[size-2]);
        	MPI_Wait(&l_send_request[size-2], &status);
        	MPI_Wait(&l_recv_request[size-2], &status);
		}
		if (rank != (size-1)) {
			// Receive from right processor (into this processor's right buffer).
			MPI_Irecv(right, EXTENT, MPI_DOUBLE, rank+1, sendL_tags[rank], MPI_COMM_WORLD,&r_recv_request[rank]);
			// Send to right processor (which receives into its left buffer).
			MPI_Isend(buffer+(chunk-EXTENT), EXTENT, MPI_DOUBLE, rank+1, sendR_tags[rank], MPI_COMM_WORLD, &r_send_request[rank]);
  	      	MPI_Wait(&r_recv_request[rank], &status);
			MPI_Wait(&r_send_request[rank], &status);
		} else {
			// Set the last processor to receive from the first.
			MPI_Irecv(right, EXTENT, MPI_DOUBLE, 1, sendL_tags[size-2], MPI_COMM_WORLD,&r_recv_request[size-2]);
			// Set the last processor to send to the first.
			MPI_Isend(buffer+(chunk-EXTENT), EXTENT, MPI_DOUBLE, 1, sendR_tags[size-2], MPI_COMM_WORLD, &r_send_request[size-2]);
  	      	MPI_Wait(&r_recv_request[size-2], &status);
			MPI_Wait(&r_send_request[size-2], &status);
		}

		for (int kappa = 0; kappa < EXTENT; kappa++) {
			printf("[PROC %d] Left value at index %d is %f:\n", rank, kappa, left[kappa]);
		}
		for (int kappa = 0; kappa < chunk; kappa++){
			printf("[PROC %d] Buffer value at index %d is %f:\n", rank, kappa, buffer[kappa]);
		}
		for (int kappa = 0; kappa < EXTENT; kappa++) {
			printf("[PROC %d] Right value at index %d is %f:\n", rank, kappa, right[kappa]);
		}

		// Repeatedly apply stencil
		for (int s=0; s<num_steps; s++) {

        // Apply stencil
		for (int i=EXTENT; i<chunk-EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {

				int index = i - EXTENT + j;
				result += STENCIL[j] * buffer[index];
			}
			output[i] = result;
		}


		        // Apply stencil on inner points
		//for (int i= (rank*chunk) + EXTENT; i<
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
	/******************************************************************************************************************************/

    }

	// Start timer
	double start = MPI_Wtime();

	/*
	// Repeatedly apply stencil
	for (int s=0; s<num_steps; s++) {

        // Apply stencil on left boundary with periodic cond
		//for (int i= rank*chunk; i<EXTENT; i++)
		for (int i=0; i<EXTENT; i++) {
			double result = 0;
			for (int j=0; j<STENCIL_WIDTH; j++) {
				int index = (i - EXTENT + j + num_values) % num_values;
				result += STENCIL[j] * input[index];
			}
			output[i] = result;
		}

        // Apply stencil on inner points
		//for (int i= (rank*chunk) + EXTENT; i<
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
	*/

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
