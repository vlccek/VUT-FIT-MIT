#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-24-108
#SBATCH -n 1
#SBATCH --comment "use:vtune=2022.2.0"
#SBATCH -t 0:10:00
#SBATCH --mail-type END
#SBATCH -J AVS-perf

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

declare -A sum

#######################################
#              PARAMETRY              #
#######################################
iter=100
iter_ref=10
threadcounts=(18 36)
builders=(loop tree)
input=bun_zipper_res3.pts
grid_size=128
#######################################
#######################################

echo "input file: ${input}"
echo "grid size: ${grid_size}"
echo "================================"

echo "iterations: ${iter} (for ref: ${iter_ref})"
echo "builder: ref"

for i in $(seq 1 ${iter_ref}); do
    output=$(./PMC --builder ref -t ${threadcounts[0]} --grid ${grid_size} ../data/${input})
    value=$(echo $output | grep -oP 'Elapsed Time:\s+\K\d+')
    sum[ref]=$((sum[ref] + value))

done

sum[ref]=$(echo "scale=2; ${sum[ref]} / ${iter_ref}" | bc)

for threads in ${threadcounts[@]}; do
    echo "  threads: ${threads}, avg: ${sum[ref]}"
done

echo ""

for builder in ${builders[@]}; do
    echo "builder: ${builder}"

    for threads in ${threadcounts[@]}; do
        sum[$threads,$builder]=0

        for i in $(seq 1 ${iter}); do
            output=$(./PMC --builder ${builder} -t ${threads} --grid ${grid_size} ../data/${input})
            value=$(echo $output | grep -oP 'Elapsed Time:\s+\K\d+')
            sum[$threads,$builder]=$((sum[$threads,$builder] + value))
        done

        sum[$threads,$builder]=$(echo "scale=2; ${sum[$threads,$builder]} / ${iter}" | bc)
        echo "  threads: ${threads}, avg: ${sum[$threads,$builder]}"
    done
    echo ""
done

echo ""
echo "performance against ref"

echo "================================"
for builder in ${builders[@]}; do
    for threads in ${threadcounts[@]}; do
        perf_ratio=$(echo "scale=5; 1-${sum[$threads,$builder]} / ${sum[ref]}" | bc)
        echo "Builder: ${builder}, Threads: ${threads}, Performance: ${perf_ratio}x"
    done
    echo ""
done
