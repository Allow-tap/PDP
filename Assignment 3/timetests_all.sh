#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p node -n 64
#SBATCH -t 1:00:00

module load gcc openmpi
export OMPI_MCA_btl_openib_allow_ib=1

EXECNAME="quicksort"
FLAGS="--bind-to none"

for seq in {0,1,2}
do
	for pivstrat in {1,2,3}
	do
		# Go to the corresponding directory.
		directory="seq$(($seq))_piv$(($pivstrat))"
		cd $directory

		### Vary number of processors, fix problem size (strong scalability).
		### Make sure the chosen input size is divisble by all the chosen
		### number of processors. 360 is a good number, since it has lots
		### of factors. (Don't ask me how I know this.)
		
		N=3600000
		NUMPROCS={1,2,4,8,16,32,64}
		TIMESFILE2="times_output_fixedinput.txt"
		
		for numproc in $( eval echo $NUMPROCS )
		do
		        echo "Number of processors = $numproc"
		        mpirun $FLAGS -np $numproc $EXECNAME $seq $N $pivstrat >> $TIMESFILE2
		done
		
		#### Vary problem size AND number of processors, but keep their ratio fixed (weak scalability)
		
		FIRSTPROC=1
		FIRSTN=10000
		TIMESFILE3="times_output_fixedratio.txt"
		MINEXPONENT=1
		MAXEXPONENT=7
		
		for exponent in $( eval echo {$MINEXPONENT..$MAXEXPONENT} )
		do
		        echo "Exponent = $exponent"
		
			nextproc=$(( $FIRSTPROC * (2**($exponent-1)) ))
		        nextN=$(( $FIRSTN * (2**($exponent-1)) ))
		
		        mpirun $FLAGS -np $nextproc $EXECNAME $seq $nextN $pivstrat >> $TIMESFILE3
		done
		
		### Vary problem size, fix number of processors.
		
		NPROC=16
		TIMESFILE1="times_output_fixedprocs.txt"
		LOWLIM=1000000
		UPLIM=10000000
		
		for N in $( eval echo {$LOWLIM..$UPLIM..$LOWLIM} )
		do
		        echo "Problem size = $N"
		
		        mpirun $FLAGS -np $NPROC $EXECNAME $seq $N $pivstrat >> $TIMESFILE1
		done

		# Go back to the previous directory for the next set of seq and/or pivstrat values.
		cd ..
	done
done

