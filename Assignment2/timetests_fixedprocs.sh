#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p node -n 20
#SBATCH -t 1:00:00

module load gcc openmpi
export OMPI_MCA_btl_openib_allow_ib=1

EXECNAME="matmul"
FLAGS="--bind-to none"

### Vary problem size, fix number of processors.

NPROC=20
TIMESFILE1="times_output_fixedprocs.txt"
LOWLIM=1
UPLIM=10

for N in {$LOWLIM..$UPLIM}
do
        echo "Problem size = $N"

	N=$(( $N*500 ))
	inputfile="input$N.txt"
	outputfile="output$N.txt"

        mpirun $FLAGS -np $NPROC $EXECNAME $inputfile $outputfile >> $TIMESFILE1
done
