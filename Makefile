## Compiler and flags

## c compiler
MPI_CC = mpicc

## C++ Compiler
MPI_PP = mpic++

## Genereal Flagse  
CFLAGS = -O2 -Wall  -std=c++11

## Profiling Flags 
PROF_FLAGS = -pg

## Hosts Flags 

MPI_HOSTS=mpi_host

## Source file
SRC = $(Pr).c

TARGET = $(Pr).out

STATIONS_COUNT = $(n)

# Build the target
all: $(TARGET)

$(TARGET): $(SRC)
	$(MPI_PP) $(CFLAGS) -o $(TARGET) $(SRC)

## Build the program 
build:
	$(MPI_PP) $(CFLAGS) -o $(TARGET) $(SRC)

## Run the program with a specified number of processes
run:
	mpirun -n STATIONS_COUNT -f  $(MPI_HOSTS) ./$(TARGET)

## Clean the build
clean:
	rm -f $(TARGET)