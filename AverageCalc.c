#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <cstdlib>
#include <ctime>

using namespace std ;


// constants and globa variables 
// Generated Matrix Size
const int MATRIX_SIZE = 1000;

// Generated Numbers Range 
const int P = 100;

// Function to intialize the numbers 
void intialize(vector<int>& generatedNumbers){
 
    // resize the vector shape 
    generatedNumbers.resize(MATRIX_SIZE);

    srand(time(0));

    for (int i = 0; i < MATRIX_SIZE; i++) {
       
        // Generate random integers between 0 and and P -1 
        generatedNumbers[i] = rand() % 100; 
    }
 
}

// Function to calculate the local sum 
int myAccumulater(vector<int>& numbers) {

    int sum = 0;

    for ( int value : numbers) {
        sum += value;
    }

    return sum;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    vector<int> generatedNumbers;

    int chunk_size = MATRIX_SIZE / size;
    int remainder = MATRIX_SIZE % size;

    if (rank < remainder) {
        chunk_size += 1; // Add extra element for processes handling remainder
    }

    vector<int> chunk(chunk_size);

    if (rank == 0) {

        // intialize the numbers 
        intialize(generatedNumbers);

        printf("[Master @ #] Root process initialized matrix\n[Master @ #] (first 10 elements): ");

        for (int i = 0; i < 10; i++) {
            printf("%d ",generatedNumbers[i] );
        }

        printf("\n[Master @ #] \n");
        
        // Divivde the work to slices 
        // each slice to a worder (Process) 
        
        // slcie offset 
        int offset = 0;
        // slice number 
        int  i  = 1;

        for ( i = 1; i < size; i++) {
            
            // calculate the slice size 
            int send_size = MATRIX_SIZE / size + (i < remainder ? 1 : 0);

            // send the data ti the process
            MPI_Send(
                generatedNumbers.data() + offset,// data
                send_size, // chunk size 
                MPI_INT, // size of chunk item
                i,0, // source and dest 
                MPI_COMM_WORLD //coomunication world
            );
            /// increasing the  slcie offset 
            offset += send_size;
        }

        // assigne a slice also to the master process s(every one should work ): !.)
        chunk.assign(generatedNumbers.begin(), generatedNumbers.begin() + chunk_size);

    } else { // workder process

        // recieving the data
        MPI_Recv(
            chunk.data(), 
            chunk_size, 
            MPI_INT, 
            0, 0, 
            MPI_COMM_WORLD, 
            MPI_STATUS_IGNORE
        );
    }
    
    // Print detailed information for each process
    int startIndex = (rank < remainder) ? rank * (MATRIX_SIZE / size + 1) : rank * (MATRIX_SIZE / size) + remainder;
    int endIndex = startIndex + chunk_size - 1;

    printf( "[Worker @ %d] I'm responsible for indices [ %d- %d ]\n",rank,startIndex,endIndex);

    // finding the local sum of our chnk 
    int localSum =myAccumulater(chunk);
    
    // calc the local average 
    int localAverage = localSum / chunk_size;
    
    printf("[Worker @ %d] Process %d calculated local average: %d \n" ,rank ,rank, localAverage);

    
    if (rank == 0) {
        
        int globalAverage = 0;
    
        vector<int> localAverages(size);

        localAverages[0] = localAverage;

        // revice the local averages from each process 
        for (int i = 1; i < size; i++) {

            MPI_Recv(
                &localAverages[i],
                1,
                MPI_INT, 
                i, 1, // SRC / DsT
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
        }

        // find the global average 
        globalAverage =myAccumulater(localAverages) / size;

       printf("[Master @ #] Global average calculated from all local averages: %d \n",globalAverage );
    } else {
        MPI_Send(&localAverage, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
