/**********************************************************************
 * Quick sort
 * Usage: ./a.out sequence length
 *
 **********************************************************************/
#define PI 3.14159265358979323846
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int partition(double *data, int left, int right, int pivotIndex);
void quicksort(double *data, int left, int right);
double *merge(double *v1, int n1, double *v2, int n2);

int main(int argc, char *argv[]) {
    
    int i,len,seq;
    
    seq=atoi(argv[1]);
    len=atoi(argv[2]);
    double *data=(double *)malloc(len*sizeof(double));
    
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
    printf("Data before sort\n");
    for (int i =0; i < len; i++){
        
        printf("%f\t",data[i]);
    }
    // Sort
    printf("\nData after sort\n");
    quicksort(data,0,len-1);
    for (int i =0; i < len; i++){
        
        printf("%f\t",data[i]);
    }
    // Check results
    int OK=1;
    for (i=0; i<len-1; i++) {
        if(data[i] > data[i+1]) {
            printf("Wrong result: data[%d] = %f, data[%d] = %f\n", i, data[i], i+1, data[i+1]);
            OK=0;
        }
    }
    
    if (OK) printf("Data sorted correctly!\n");
    free(data);
    
    return 0;
}

int partition(double *data, int left, int right, int pivotIndex){
    double pivotValue,temp;
    int storeIndex,i;
    pivotValue = data[pivotIndex];
    temp=data[pivotIndex]; data[pivotIndex]=data[right]; data[right]=temp;
    storeIndex = left;
    for (i=left;i<right;i++)
    if (data[i] <= pivotValue){
        temp=data[i];data[i]=data[storeIndex];data[storeIndex]=temp;
        storeIndex = storeIndex + 1;
    }
    temp=data[storeIndex];data[storeIndex]=data[right]; data[right]=temp;
    return storeIndex;
}

void quicksort(double *data, int left, int right){
    int pivotIndex, pivotNewIndex;
    
    if (right > left){
        pivotIndex = left+(right-left)/2;
        pivotNewIndex = partition(data, left, right, pivotIndex);
        quicksort(data,left, pivotNewIndex - 1);
        quicksort(data,pivotNewIndex + 1, right);
    }
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
