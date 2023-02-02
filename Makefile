# Octombrie 2021

build:
	@echo "Building..."
	@gcc -o knapsack_par par_knapsack.c par_genetic_algorithm.c -lm -Wall -Werror -pthread
	@echo "Done"

build_debug:
	@echo "Building debug..."
	@gcc -o knapsack_par par_knapsack.c par_genetic_algorithm.c -lm -Wall -Werror -pthread -O0 -g -DDEBUG
	@echo "Done"

clean:
	@echo "Cleaning..."
	@rm -rf knapsack_par
	@echo "Done"
