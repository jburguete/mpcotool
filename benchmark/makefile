flags = -Wall -c -O3 
libs = -lm

cc = gcc -march=native -mfpmath=sse -Wall #-pg

simulator = simulator
evaluator = evaluator

all: $(simulator) $(evaluator)

$(simulator): simulator.c makefile
	$(cc) simulator.c -lm -o $(simulator)

$(evaluator): evaluator.c makefile
	$(cc) evaluator.c -lm -o $(evaluator)

