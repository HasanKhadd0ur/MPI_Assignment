#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std; 

// Naïve implementation of MPI_Allreduce
void naive_allreduce(int local_value, int& global_value, MPI_Comm comm) {

    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // to store values from all processes
    vector<int> all_values(size, 0);
    
    // [1] Send the local value to all processes
 
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            MPI_Send(&local_value, 1, MPI_INT, i, 0, comm);
        }
    }

    // [2] Receive values from all processes

    // Store own value first
    all_values[rank] = local_value; 
    // Recieve the values from other process
    for (int i = 0; i < size; i++) {
        
        // to avoid recive value from the process itself 
        if (i != rank) {
            // Revieve the value gfrom rnal i 
            MPI_Recv(
                &all_values[i],
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
    global_value = 0;

    for (int i = 0; i < size; i++) {
        //reduction 
        global_value += all_values[i];
    }
}

void naive_allreduce_array(const vector<int>& local_array, vector<int>& global_array, MPI_Comm comm) {
    // init 
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    int array_size = local_array.size();
    
    // Temporary buffer to store received arrays
    vector<int> received_array(array_size, 0); 

    // Initialize global_array with local_array
    global_array = local_array;

    // [1] Exchange data with all other processes
    for (int i = 0; i < size; i++) {
        if (i != rank) {
            // Send local_array to process i
            MPI_Send(local_array.data(), array_size, MPI_INT, i, 0, comm);

            // Receive array from process i
            MPI_Recv(received_array.data(), array_size, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);

            // Add the received array to the global_array
            for (int j = 0; j < array_size; j++) {
                global_array[j] += received_array[j];
            }
        }
    }
}

void print(vector<int>& array ,int rank, string message){

    string output_local = "[Worker @ " +to_string(rank) + "] "+message+": ";

    for (int val : array) {

        output_local += to_string(val) + " ";
    }

    printf("%s \n", output_local.c_str());

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
    int local_value = rand() % 100; 

    const int ARRAY_SIZE = 5; // Length of each process's array
    vector<int> local_array(ARRAY_SIZE);
    vector<int> global_array_naive(ARRAY_SIZE);
    vector<int> global_array_builtin(ARRAY_SIZE);

    // Generate random integers for the local array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        local_array[i] = rand() % 100; // Random integers between 0 and 99
    }


    // Print local values for each process
    // printf("[Worker @ %d] I has local value: %d \n",rank,local_value );

    print(local_array,rank,"Local array");


    //[1] Use the naive implementation of MPI_Allreduce
    int global_value_naive = 0;
    
    // Start timing
    double start_time_naive = MPI_Wtime(); 
    // naive MPI All Reduce 
    // naive_allreduce(
    //     local_value, 
    //     global_value_naive, 
    //     MPI_COMM_WORLD
    // );
    
    naive_allreduce_array(
        local_array, 
        global_array_naive, 
        MPI_COMM_WORLD
    );
 
     // End timing
    double end_time_naive = MPI_Wtime();
    double time_naive = end_time_naive - start_time_naive;

    // printf("[Worker @ %d] My Naive MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
    //        rank, global_value_naive, time_naive);

    string message ="My Naive MPI_Allreduce "; 
    message += "Time: " + to_string(time_naive) + " seconds | Result";


    print(global_array_naive,rank,message);

    // [2] Use the built-in MPI_Allreduce for comparison
    int global_value_builtin = 0;
    
    // Start timing
    double start_time_builtin = MPI_Wtime(); 
    // Mpi Alllreduce buld in
    // MPI_Allreduce(
    //     &local_value, 
    //     &global_value_builtin, 
    //     1, MPI_INT, 
    //     MPI_SUM, 
    //     MPI_COMM_WORLD
    // );

    MPI_Allreduce(
        local_array.data(), 
        global_array_builtin.data(), 
        ARRAY_SIZE, 
        MPI_INT, 
        MPI_SUM, 
        MPI_COMM_WORLD
    );
  
    // End timing
    double end_time_builtin = MPI_Wtime(); 
    double time_builtin = end_time_builtin - start_time_builtin;

    // printf("[Worker @ %d] Built-in MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
    //        rank, global_value_builtin, time_builtin);

    message ="Built-in MPI_Allreduce"; 
    message += " Time: " + to_string(time_builtin) + " seconds | Result";

    print(global_array_builtin,rank,message);

    // Fina;ize 
    MPI_Finalize();
    return 0;
}
