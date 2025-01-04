#!/bin/bash

# Create directories for output
mkdir -p profiling_results
mkdir -p compiled_bins

# Compile all three versions with profiling flags
g++ -pg -g -O2 q2.cpp -o compiled_bins/hull_vector
g++ -pg -g -O2 q2_list.cpp -o compiled_bins/hull_list
g++ -pg -g -O2 q2_deque.cpp -o compiled_bins/hull_deque

# Function to generate random test data
generate_test_data() {
    local size=$1
    local filename=$2
    echo $size > $filename
    for ((i=0; i<size; i++)); do
        echo "$RANDOM,$RANDOM" >> $filename
    }
}

# Generate test datasets of different sizes
TEST_SIZES=(100 1000 10000 100000)
for size in "${TEST_SIZES[@]}"; do
    generate_test_data $size "profiling_results/input_${size}.txt"
done

# Function to run valgrind analysis
run_valgrind() {
    local binary=$1
    local input_file=$2
    local size=$3
    local impl=$4
    
    echo "Running Valgrind for ${impl} implementation with ${size} points..."
    
    valgrind --tool=massif --massif-out-file="profiling_results/massif_${impl}_${size}.out" \
        ./$binary < $input_file > /dev/null 2>&1
    
    ms_print "profiling_results/massif_${impl}_${size}.out" > "profiling_results/memory_${impl}_${size}.txt"
    
    valgrind --tool=callgrind --callgrind-out-file="profiling_results/callgrind_${impl}_${size}.out" \
        ./$binary < $input_file > /dev/null 2>&1
        
    callgrind_annotate "profiling_results/callgrind_${impl}_${size}.out" > "profiling_results/cpu_${impl}_${size}.txt"
}

# Function to run time benchmarks
run_time_benchmark() {
    local binary=$1
    local input_file=$2
    local size=$3
    local impl=$4
    local iterations=5
    
    echo "Running time benchmark for ${impl} implementation with ${size} points..."
    
    echo "Time measurements for ${impl} (${size} points):" > "profiling_results/time_${impl}_${size}.txt"
    for ((i=1; i<=iterations; i++)); do
        { time ./$binary < $input_file > /dev/null; } 2>> "profiling_results/time_${impl}_${size}.txt"
    done
}

# Run profiling for each implementation and dataset
IMPLEMENTATIONS=("vector" "list" "deque")
for size in "${TEST_SIZES[@]}"; do
    for impl in "${IMPLEMENTATIONS[@]}"; do
        run_valgrind "compiled_bins/hull_${impl}" "profiling_results/input_${size}.txt" "$size" "$impl"
        run_time_benchmark "compiled_bins/hull_${impl}" "profiling_results/input_${size}.txt" "$size" "$impl"
    done
done

# Generate summary report
echo "Generating summary report..."
{
    echo "Convex Hull Implementation Comparison"
    echo "===================================="
    echo
    echo "Test conducted on $(date)"
    echo
    
    for size in "${TEST_SIZES[@]}"; do
        echo "Dataset size: $size points"
        echo "------------------------"
        
        for impl in "${IMPLEMENTATIONS[@]}"; do
            echo
            echo "${impl} implementation:"
            echo "Peak memory usage (from massif):"
            grep "peak=" "profiling_results/memory_${impl}_${size}.txt" | head -n 1
            
            echo "Execution times:"
            grep "real" "profiling_results/time_${impl}_${size}.txt"
            
            echo "Top 3 hotspots (from callgrind):"
            head -n 20 "profiling_results/cpu_${impl}_${size}.txt" | grep -A 3 "Percent" | tail -n 3
            echo
        done
        echo "===================================="
    done
} > profiling_results/summary_report.txt

echo "Profiling complete! Results are in the profiling_results directory."
echo "See profiling_results/summary_report.txt for a summary of all results."