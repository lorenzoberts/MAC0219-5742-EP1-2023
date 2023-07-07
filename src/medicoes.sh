grid_sizes=(32 64 128 256 512 1024 2048 4096)
num_threads=(1 2 4 8 16 32)
num_medicoes=10
impls=("seq" "pth" "omp")

OUTPUT_FILE="resultado_medicoes.csv"


csv_out=$'impl,grid_size,num_threads'
for i in $(seq 1 $num_medicoes); do
	csv_out="$csv_out","measure_$i"
done
csv_out="$csv_out"$'\n'

for impl in ${impls[@]}; do
    for g_size in ${grid_sizes[@]}; do
        for n_threads in ${num_threads[@]}; do

        	csv_line="$impl"','"$g_size"','"$n_threads"

        	for m in $(seq 1 $num_medicoes); do
        		resultado=$(./time_test --grid_size $g_size --impl $impl --num_threads $n_threads)
        		csv_line="$csv_line"','"$resultado"
        	done
        	
        	csv_line="$csv_line"$'\n'

        	csv_out="$csv_out""$csv_line"
        done
    done
    echo "Finished measures for $impl"
done

echo "$csv_out" > $OUTPUT_FILE