#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std; 

// Naïve implementation of MPI_Allreduce
void naiveAllreduce(int localValue, int& globalValue, MPI_Comm comm) {

    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // to store values from all processes
    vector<int> allValues(size, 0);
    
    // [1] Send the local value to all processes
 
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(&localValue, 1, MPI_INT, i, 0, comm);
        }
    }

    // [2] Receive values from all processes

    // Store own value first
    allValues[rank] = localValue; 
    // Recieve the values from other process
    for (int i = 0; i < size; i++) {
        
        // to avoid recive value from the process itself 
        if (i != rank) {
            // Revieve the value gfrom rnal i 
            MPI_Recv(
                &allValues[i],
                1, 
                MPI_INT, 
                i, 0, 
                comm, 
                MPI_STATUS_IGNORE
            );
        }
    }

    // [3] Perform reduction (sum)

    // global value 
    globalValue = 0;

    for (int i = 0; i < size; i++) {
        //reduction 
        globalValue += allValues[i];
    }
}

void naiveAllreduceArray(const vector<int>& localArray, vector<int>& globalArray, MPI_Comm comm) {
    // init 
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    int arraySize = localArray.size();
    
    // Temporary buffer to store received arrays
    vector<int> receivedArray(arraySize, 0); 

    // Initialize global_array with localArray
    globalArray = localArray;

    // [1] Exchange data with all other processes
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            // Send localArray to process i
            MPI_Send(localArray.data(), arraySize, MPI_INT, i, 0, comm);

            // Receive array from process i
            MPI_Recv(receivedArray.data(), arraySize, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);

            // Add the received array to the global_array
            for (int j = 0; j < arraySize; j++) {
                globalArray[j] += receivedArray[j];
            }
        }
    }
}

void print(vector<int>& array ,int rank, string message){

    string output = "[Worker @ " +to_string(rank) + "] "+message+": ";

    for (int val : array) {

        output += to_string(val) + " ";
    }

    printf("%s \n", output.c_str());

}

int main(int argc, char** argv) {

    // init 
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize local values

    // Seed with time and rank for unique values
    srand(time(0) + rank); 

    // Random integer between 0 and 99 (this is first simple intutyion in the next stage i will replace it with array )
    //int localValue = rand() % 100; 

    const int ARRAY_SIZE = 5; // Length of each process's array
    vector<int> localArray(ARRAY_SIZE);
    vector<int> globalArrayNaive(ARRAY_SIZE);
    vector<int> globalArrayBuiltin(ARRAY_SIZE);

    // Generate random integers for the local array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        localArray[i] = rand() % 100; // Random integers between 0 and 99
    }


    // Print local values for each process
    // printf("[Worker @ %d] I has local value: %d \n",rank,localValue );

    print(localArray,rank,"Local array");


    //[1] Use the naive implementation of MPI_Allreduce
   // int globalValue = 0;
    
    // Start timing
    double start_time_naive = MPI_Wtime(); 
    // naive MPI All Reduce 
    // naive_allreduce(
    //     localValue, 
    //     globalValue, 
    //     MPI_COMM_WORLD
    // );
    
    naiveAllreduceArray(
        localArray, 
        globalArrayNaive, 
        MPI_COMM_WORLD
    );
 
     // End timing
    double end_time_naive = MPI_Wtime();
    double time_naive = end_time_naive - start_time_naive;

    // printf("[Worker @ %d] My Naive MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
    //        rank, globalValue, time_naive);

    string message ="My Naive MPI_Allreduce "; 
    message += "Time: " + to_string(time_naive) + " seconds | Result";


    print(globalArrayNaive,rank,message);

    // [2] Use the built-in MPI_Allreduce for comparison
   // int globalValueBuiltIn = 0;
    
    // Start timing
    double start_time_builtin = MPI_Wtime(); 
    // Mpi Alllreduce buld in
    // MPI_Allreduce(
    //     &localValue, 
    //     &globalArrayBuiltin, 
    //     1, MPI_INT, 
    //     MPI_SUM, 
    //     MPI_COMM_WORLD
    // );

    MPI_Allreduce(
        localArray.data(), 
        globalArrayBuiltin.data(), 
        ARRAY_SIZE, 
        MPI_INT, 
        MPI_SUM, 
        MPI_COMM_WORLD
    );
  
    // End timing
    double end_time_builtin = MPI_Wtime(); 
    double time_builtin = end_time_builtin - start_time_builtin;

    // printf("[Worker @ %d] Built-in MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
    //        rank, globalArrayBuiltin, time_builtin);

    message ="Built-in MPI_Allreduce"; 
    message += " Time: " + to_string(time_builtin) + " seconds | Result";

    print(globalArrayBuiltin,rank,message);

    // Fina;ize 
    MPI_Finalize();
    return 0;
}
