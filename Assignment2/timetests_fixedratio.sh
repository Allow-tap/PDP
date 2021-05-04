#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p node -n 20
#SBATCH -t 1:00:00

module load gcc openmpi
export OMPI_MCA_btl_openib_allow_ib=1

EXECNAME="matmul"
FLAGS="--bind-to none"

#### Vary problem size AND number of processors, but keep their ratio fixed (weak scalability)

FIRSTPROC=1
FIRSTN=300
TIMESFILE3="times_output_fixedratio.txt"
MINFACTOR=1
MAXFACTOR=20

for factor in $( eval echo {$MINFACTOR..$MAXFACTOR} )
do
        echo "Factor = $factor"

        nextproc=$(( $FIRSTPROC*$factor ))
        nextN=$(( $FIRSTN*$factor ))
	inputfile="input$nextN.txt"
	outputfile="output$nextN.txt"

        mpirun $FLAGS -np $nextproc $EXECNAME $inputfile $outputfile >> $TIMESFILE3
done

