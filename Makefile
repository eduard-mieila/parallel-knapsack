# APD - Tema 1
# Octombrie 2021

build:
	@echo "Building..."
	@gcc -o tema1_par par_tema1.c par_genetic_algorithm.c -lm -Wall -Werror -pthread
	@echo "Done"

build_debug:
	@echo "Building debug..."
	@gcc -o tema1_par par_tema1.c par_genetic_algorithm.c -lm -Wall -Werror -pthread -O0 -g -DDEBUG
	@echo "Done"

clean:
	@echo "Cleaning..."
	@rm -rf tema1_par
	@echo "Done"
