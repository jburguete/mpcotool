flags = -Wall -O3 -DHAVE_MPI=1 `pkg-config gsl glib-2.0 --cflags`
libs = -lm `pkg-config gsl glib-2.0 --libs`
objs = ../../genetic/genetic.o ../../genetic/evolution.o \
	../../genetic/selection.o ../../genetic/reproduction.o \
	../../genetic/population.o ../../genetic/entity.o

cc = mpicc -march=native -mfpmath=sse -Wall #-pg

simulator = simulator

all: $(simulator) $(evaluator)

$(simulator): simulator.c makefile
	$(cc) $(flags) simulator.c $(objs) $(libs) -o $(simulator)
