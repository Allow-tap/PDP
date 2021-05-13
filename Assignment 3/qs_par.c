#define PI 3.14159265358979323846
#define _XOPEN_SOURCE
#define STRAT_ONE ((int)1)
#define STRAT_TWO ((int)2)
#define STRAT_THREE ((int)3)
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

/* Forward Declaration */
double* gen_num_vector(int seq, int len);
double* merge(double *v1, int n1, double *v2, int n2);
int cmpfunc (const void * a, const void * b);
double find_pivot(double* data, int len);
double find_mean(double* data, int len);
void check_pivot(double* data, int len, double pivot, int rank);
double pivot(int pivot_strat,int len, double *rcv_buffer, int rank, int size, MPI_Comm comm);

int main(int argc, char **argv) {

    int size, rank,len, seq, pivot_strat;
    double *data, *rcv_buffer, g_pivot;
    /* Initializing MPI */
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    
    seq=atoi(argv[1]);
    len=atoi(argv[2]);
    pivot_strat = atoi(argv[3]);
    data = gen_num_vector(seq,len);

    /*** 1. Divide and distribute the data into p equal parts, one per process ***/
    int chunk = len / size;
    
    /* Prints the values that we send */
    /*
    if (rank == 0){
        
        for (int i =0; i < len; ++i)
            printf("Element %d: [%f]\n",i, data[i]);  
    }   
    */

    rcv_buffer = (double*)malloc(chunk*sizeof(double));
    MPI_Scatter(data, chunk, MPI_DOUBLE, rcv_buffer, chunk, MPI_DOUBLE,0,MPI_COMM_WORLD);
    
    /* Prints the values each PROC receives */
    //for (int i =0; i < chunk; ++i)
    //    printf("RANK[%d]\tdata[%d]=[%f]\n",rank, i, rcv_buffer[i]); 
    
    /*** 2. Sort the data locally for each process ***/
    qsort(rcv_buffer, chunk, sizeof(double), cmpfunc); /* qsort() from stdlib does that for us */

    /* Check that qsort() is successfull */
    /*Prints the values each PROC after local sort*/
    /*
    for (int i =0; i < chunk; ++i)
        printf("RANK[%d]\tElement %d: [%f]\n",rank, i, rcv_buffer[i]); 
    */

    /* Find the global Pivot  */
    g_pivot = pivot(pivot_strat, chunk, rcv_buffer, rank, size, MPI_COMM_WORLD);
    
    /*** 3. Perform global sort  ***/

    //Quicksort is recursive so it needs to be a function
    //parallel_quicksort(MPI_Comm comm, int rank, int size, double* data, int chunk);




    check_pivot(data,len,g_pivot,rank);
    free(rcv_buffer);
    free(data);
    MPI_Finalize();
    return 0;
}

double* parallel_quicksort(MPI_Comm comm, int rank, int size, double* data, int chunk){

}

double pivot(int pivot_strat,int len, double *rcv_buffer, int rank, int size, MPI_Comm comm){
    double g_pivot = 0;
    switch (pivot_strat){
        case STRAT_ONE : /* 1. Select the median in one processor in each group of processors*/
        {
            /* ROOT find the Global PIVOT and broadcast it to all other PROCS */
            if (rank==0){
                g_pivot = find_pivot(rcv_buffer, len);
                //g_pivot = find_mean(data, len);
                
                //for (int i =0; i < chunk; ++i)
                //    printf("RANK[%d]\tElement %d: [%f]\n",rank, i, data[i]); 
                
                //printf("Global Pivot [%f]\n", g_pivot);
                
            }
            MPI_Bcast ( &g_pivot, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            //printf("Global Pivot [%f]", g_pivot);
        }
        break;
        case STRAT_TWO : /* Select the median of all medians in each processor group*/
        {
            double local_pivot = find_pivot(rcv_buffer, len); /* Each PROC find median of local buffer */
            double all_median[size]; /* # of median values as our #PROCS */
            MPI_Allgather( &local_pivot, 1, MPI_DOUBLE, all_median, 1, MPI_DOUBLE, MPI_COMM_WORLD);
            g_pivot = find_pivot(all_median, size); /* median of all medians */
            //printf("Global Pivot [%f]\n", g_pivot);
        }
        break;
        case STRAT_THREE: /* Select the mean value of all medians in each processor group */
        {
            double local_pivot = find_pivot(rcv_buffer, len); /* Each PROC find median of local buffer */
            double all_median[size]; /* # of median values as our #PROCS */
            MPI_Allgather( &local_pivot, 1, MPI_DOUBLE, all_median, 1, MPI_DOUBLE, MPI_COMM_WORLD);
            g_pivot = find_mean( all_median, size);
            //printf("Global Pivot [%f]\n", g_pivot);
        }
        break;
    }
    return g_pivot;
}

double find_pivot(double* data, int len){
    /* If the number of elements is zero the pivot is data[0] */
    if (len==0)
        return 0;
    double pivot = 0;
    /* If number of elements is odd, that pivot is the middle element */
    if (len % 2 != 0)
        pivot = data[len/2];
    /* Else we set it to be equal to half the sum of two "middle" elements */
    else
        pivot =  ((double)data[(len / 2) -1] + (double)data[(len / 2)]) / 2.0;
    return pivot;
}

double find_mean(double *data, int len){
    double sum = 0;
    for (int i=0; i<len; ++i)
        sum += data[i];
    sum = sum/(double)len;
    return sum;
}

void check_pivot(double* data, int len, double pivot, int rank){
    int lower,upper;    
    for (int i=0; i<len; i++){
        if (data[i] > pivot)
            upper++;
        else
            lower++;
        
    }
    if (rank == 0)
        printf("#lower=%d\t#upper=%d\n", lower, upper);
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

int cmpfunc (const void * a, const void * b)
{
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
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
