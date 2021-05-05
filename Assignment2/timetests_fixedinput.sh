#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p node -n 60
#SBATCH -t 1:00:00

module load gcc openmpi
export OMPI_MCA_btl_openib_allow_ib=1

EXECNAME="matmul"
FLAGS="--bind-to none"

### Vary number of processors, fix problem size (strong scalability).
### Make sure the chosen input size is divisble by all the chosen
### number of processors. 360 is a good number, since it has lots
### of factors. (Don't ask me how I know this.)

N=3600
INPUTFILE="input$N.txt"
OUTPUTFILE="output$N.txt"
NUMPROCS={1,2,3,4,5,6,8,9,10,12,15,18,20,24,30,36,40,45,60}
TIMESFILE2="times_output_fixedinput.txt"

for numproc in $( eval echo $NUMPROCS )
do
        echo "Number of processors = $numproc"
        mpirun $FLAGS -np $numproc $EXECNAME $INPUTFILE $OUTPUTFILE >> $TIMESFILE2
done
