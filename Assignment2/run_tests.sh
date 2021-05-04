#!/bin/bash -l

#SBATCH -A uppmax2021-2-7
#SBATCH -p core -n 2
#SBATCH -t 5:00

module load gcc openmpi
mpirun -np 2 matmul 