# Makefile for profiling vector and deque implementations

CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
PROFFLAGS = -pg -g

# Standard compilation
vector: ch_vector.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

deque: ch_deque.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

all: vector deque

# Profiling versions
vector_prof: ch_vector.cpp
	$(CXX) $(CXXFLAGS) $(PROFFLAGS) -o $@ $<

deque_prof: ch_deque.cpp
	$(CXX) $(CXXFLAGS) $(PROFFLAGS) -o $@ $<

all_prof: vector_prof deque_prof

# Generate test data
generate_data:
	@echo "Generating test data with 1,000,000 points..."
	@echo "1000000" > test_data.txt
	@for i in $$(seq 1 1000000); do \
    		x=$$(( i % 830 )); \
    		y=$$(( i % 365 )); \
    		echo "$$x,$$y" >> test_data.txt; \
    done
	@echo "Generated test data with 10000 points in test_data.txt"

# Generate test data with custom size
generate_custom:
	@read -p "Enter number of points: " n; \
	echo "$$n" > test_data.txt; \
	for i in $$(seq 1 $$n); do \
		echo "$$(( RANDOM % 1000 )),$$(( RANDOM % 1000 ))" >> test_data.txt; \
	done; \
	echo "Generated test data with $$n points in test_data.txt"

# Run profiling tests
profile_vector: vector_prof
	@echo "Profiling vector implementation..."
	./vector_prof < test_data.txt
	gprof vector_prof gmon.out > profile_vector.txt
	@echo "Vector profiling report generated as profile_vector.txt"

profile_deque: deque_prof
	@echo "Profiling deque implementation..."
	./deque_prof < test_data.txt
	gprof deque_prof gmon.out > profile_deque.txt
	@echo "Deque profiling report generated as profile_deque.txt"

profile_all: vector_prof deque_prof
	@echo "Profiling vector implementation..."
	./vector_prof < test_data.txt
	gprof vector_prof gmon.out > profile_vector.txt

	@echo "Profiling deque implementation..."
	./deque_prof < test_data.txt
	gprof deque_prof gmon.out > profile_deque.txt

	@echo "All profiling reports generated."

# Compare execution times without profiling
compare: vector deque
	@echo "\nComparing implementations with the same input data..."
	@echo "\nVector implementation:"
	time ./vector < test_data.txt

	@echo "\nDeque implementation:"
	time ./deque < test_data.txt

# Clean up
clean:
	rm -f vector deque vector_prof deque_prof gmon.out profile_*.txt test_data.txt

.PHONY: all all_prof clean compare generate_data generate_custom profile_vector profile_deque profile_all