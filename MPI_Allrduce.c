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

    // Print local values for each process
   printf("[Worker @ %d] I has local value: %d \n",rank,local_value );

    //[1] Use the naive implementation of MPI_Allreduce
    int global_value_naive = 0;
    
    // Start timing
    double start_time_naive = MPI_Wtime(); 
    // naive MPI All Reduce 
    naive_allreduce(
        local_value, 
        global_value_naive, 
        MPI_COMM_WORLD
    );
     // End timing
    double end_time_naive = MPI_Wtime();
    double time_naive = end_time_naive - start_time_naive;

    printf("[Worker @ %d] My Naive MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
           rank, global_value_naive, time_naive);

    // [2] Use the built-in MPI_Allreduce for comparison
    int global_value_builtin = 0;
    
    // Start timing
    double start_time_builtin = MPI_Wtime(); 
    // Mpi Alllreduce buld in
    MPI_Allreduce(
        &local_value, 
        &global_value_builtin, 
        1, MPI_INT, 
        MPI_SUM, 
        MPI_COMM_WORLD
    );
    // End timing
    double end_time_builtin = MPI_Wtime(); 
    double time_builtin = end_time_builtin - start_time_builtin;

    printf("[Worker @ %d] Built-in MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
           rank, global_value_builtin, time_builtin);

    // Fina;ize 
    MPI_Finalize();
    return 0;
}
