#define PI 3.14159265358979323846
#define _XOPEN_SOURCE
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

/* Forward Declaration */
double* gen_num_vector(int seq, int len);
double *merge(double *v1, int n1, double *v2, int n2);

int main(int argc, char **argv) {

    /* Initializing MPI */
    int size, rank, pivotStrategy, len, seq;
    double *data;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    
    seq=atoi(argv[1]);
    len=atoi(argv[2]);
    //pivotStrategy = atoi(argv[3]);
    data = gen_num_vector(seq,len);
 
    if ( data != NULL)
        printf("SUCCESS\n");
    for (int i =0; i < len; ++i)
        printf("Element %d: [%f]\n",i, data[i]);
    /* Split the data equally among the #PROCS */
    
    free(data);
    MPI_Finalize();
    return 0;
    
}


double* gen_num_vector(int seq, int len){
    
    double *data=(double *)malloc(len*sizeof(double));
    int i;
    if (seq==0) {
        
        // Uniform random numbers
        for (i=0;i<len;i++)
        data[i]=drand48();
        
    }
    
    else if (seq==1) {
        
        // Exponential distribution
        double lambda=10;
        for (i=0;i<len;i++)
        data[i]=-lambda*log(1-drand48());
    }
    
    else if (seq==2) {
        
        // Normal distribution
        double x,y;
        for (i=0;i<len;i++){
            x=drand48(); y=drand48();
            data[i]=sqrt(-2*log(x))*cos(2*PI*y);
        }
    }
    return data;
}


double *merge(double *v1, int n1, double *v2, int n2)
{
    int i,j,k;
    double *result;
    
    result = (double *)malloc((n1+n2)*sizeof(double));
    
    i=0; j=0; k=0;
    while(i<n1 && j<n2)
        if(v1[i]<v2[j])
        {
            result[k] = v1[i];
            i++; k++;
        }
        else
        {
            result[k] = v2[j];
            j++; k++;
        }
    if(i==n1)
        while(j<n2)
        {
            result[k] = v2[j];
            j++; k++;
        }
    else
        while(i<n1)
        {
            result[k] = v1[i];
            i++; k++;
        }
    return result;
}