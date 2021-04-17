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
    
    MPI_Init(&argc, &argv);

	// Generate values for stencil operation
	double *input=(double*)malloc(num_values*sizeof(double));
    double h = 2.0*PI/num_values;
    for (int i=0; i<num_values; i++) input[i]=sin(h*i);

	// Stencil values
	const int STENCIL_WIDTH = 5;
	const int EXTENT = STENCIL_WIDTH/2;
	const double STENCIL[] = {1.0/(12*h), -8.0/(12*h), 0.0, 8.0/(12*h), -1.0/(12*h)};

	// Start timer
	double start = MPI_Wtime();

    // Allocate data for result
    double *output =(double*)malloc(num_values*sizeof(double));
	
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
    printf("%f\n", my_execution_time);
    
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
