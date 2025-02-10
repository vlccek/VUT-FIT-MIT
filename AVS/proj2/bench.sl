#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-24-108
#SBATCH -n 1 
#SBATCH --comment "use:vtune=2022.2.0"
#SBATCH -t 0:15:00
#SBATCH --mail-type END
#SBATCH -J AVS-bench

cd $SLURM_SUBMIT_DIR

ml CMake/3.27.6-GCCcore-13.2.0 intel-compilers/2024.2.0 

export OMP_PROC_BIND=close 
export OMP_PLACES=cores

[ -d build_bench ] && rm -rf build_bench
[ -d build_bench ] || mkdir build_bench
cd build_bench

CC=icx CXX=icpx cmake ..
make

echo
echo

sum=()
iter=20
threadcounts=(18 36)
builders=(loop tree)
inputs=(../data/bun_zipper_res3  ../data/bun_zipper_res2 ../data/bun_zipper_res4 ../data/bun_zipper_res1)

for builder in ${builders[@]}; do
    for input in ${inputs[@]}; do
        for threads in ${threadcounts[@]}; do
            sum[$threads]=0
            count=0

            for i in $(seq 1 ${iter}); do
                output=$(./PMC --builder ${builder} -t ${threads} --grid 128 ${input}.pts)
                value=$(echo $output | grep -oP 'Elapsed Time:\s+\K\d+')
                sum[$threads]=$((sum[$threads] + value))
                ((count++))
            done

            sum[$threads]=$((sum[$threads] / count))
        done

        echo "iterations: ${iter}"
        echo "builder: ${builder}"
        echo "input: ${input}"

        for threads in ${threadcounts[@]}; do
            echo "threads: ${threads}, avg: ${sum[$threads]} ms"
        done
    done
done
