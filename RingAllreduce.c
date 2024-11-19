#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std; 

// Length of each processs array (array chunk)
const int ARRAY_SIZE = 5; 

// Ring Allreduce implementation for arrays
void ringAllreduce(const vector<int>& localArray, vector<int>& globalArray, MPI_Comm comm) {

    // world info 
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // aracy size 
    int arraySize = localArray.size();
    
    // initialize global array with the local values    
    globalArray = localArray; 

    // temporarly buffers for sending and receiving data
    vector<int> sendBuffer = globalArray;
    vector<int> recvBuffer(arraySize, 0);

    // [1] first Phase :
    //   Share and Reduce phase 
    //   in this phase each process send and revcive data chinl from next and previous process and reduce it 
    for (int step = 0; step < size - 1; ++step) {

        // next process in the ring
        int sendTo = (rank + 1) % size;     
        
        // previous process in the ring
        int recvFrom = (rank - 1 + size) % size; 

        // send and receive arrays
        //  i use this as a send and recive as a one command
        // beacuse when using the as separated we should wait for boh as shown below
        // MPI_Sendrecv(send_buffer.data(), array_size, MPI_INT, send_to, 0,
        //              recv_buffer.data(), array_size, MPI_INT, recv_from, 0,
        //              comm, MPI_STATUS_IGNORE);
        // define requests 
        MPI_Request requests[2];

        // imediate sen d
        MPI_Isend(sendBuffer.data(), arraySize, MPI_INT, sendTo, 0, comm, &requests[0]);

        // imediate recive 
        MPI_Irecv(recvBuffer.data(), arraySize, MPI_INT, recvFrom, 0, comm, &requests[1]);

        // waiting unit the two request been completed 
        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

        // showing a usefull info about what happeining 
         printf("[Worker @ %d] Step %d Share-Reduce Phase: Sent to %d, Received from %d\n",
               rank, step, sendTo, recvFrom);
    
        // element wise reduction (sum) (maybe we can make it more generic )
        for (int i = 0; i < arraySize; ++i) {
            globalArray[i] += recvBuffer[i];
        }

        // prepare the send buffer for the next iteration
        sendBuffer = recvBuffer;
    }

    //[2] secondd Phase :
    //  Share-Only Phase 
    // in this phase we consolidates the result of each chunk in every process
    
    // start sharing the reduced array
    sendBuffer = globalArray; 

    for (int step = 0; step < size - 1; ++step) {
    
        // next process in the ring
        int sendTo = (rank + 1) % size;   
  
        // previous process in the ring
        int recvFrom = (rank - 1 + size) % size;

        // send and receive arrays
        
        // Define the requests 
        MPI_Request requests[2];
        // imediate send 
        MPI_Isend(sendBuffer.data(), arraySize, MPI_INT, sendTo, 0, comm, &requests[0]);
        // iomdeate revive 
        MPI_Irecv(recvBuffer.data(), arraySize, MPI_INT, recvFrom, 0, comm, &requests[1]);

        // waiting for both operations to complete
        MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);

        // helpfull message 
        printf("[Worker @ %d] Step %d Share-Only: Sent to %d, Received from %d\n",
               rank, step, sendTo, recvFrom);
       
        // update the global array with the received data
        globalArray = recvBuffer;
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

    // initialize local values

    // seed with time and rank for unique values
    srand(time(0) + rank); 

    // random integer between 0 and 99 (this is first simple intutyion in the next stage i will replace it with array )
    int localValue = rand() % 100; 

    // define a :
    // local array for the local reduction 
    vector<int> localArray(ARRAY_SIZE);
    // global array for the ring result 
    vector<int> globalArrayRing(ARRAY_SIZE);
    // global array for the build result 
    vector<int> globalArrayBuiltin(ARRAY_SIZE);

    // generate random integers for the local array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        localArray[i] = rand() % 100; // Random integers between 0 and 99
    }


    // Print local values for each process
    // printf("[Worker @ %d] I has local value: %d \n",rank,local_value );

    print(localArray,rank,"Local array");


    //[1] Use the rinf implementation of MPI_Allreduce

    // Start timing
    double start_time_naive = MPI_Wtime(); 
    
    ringAllreduce(
        localArray, 
        globalArrayRing, 
        MPI_COMM_WORLD
    );
 
     // End timing
    double end_time_naive = MPI_Wtime();
    double time_naive = end_time_naive - start_time_naive;

    // printf("[Worker @ %d] My Naive MPI_Allreduce: Global sum = %d, Time = %.6f seconds\n",
    //        rank, global_value_naive, time_naive);

    string message ="Our Ring MPI_Allreduce "; 
    message += "Time: " + to_string(time_naive) + " seconds | Result";


    print(globalArrayRing,rank,message);

    // [2] Use the built-in MPI_Allreduce for comparison
    
    // start timing
    double start_time_builtin = MPI_Wtime(); 

    MPI_Allreduce(
        localArray.data(), 
        globalArrayBuiltin.data(), 
        ARRAY_SIZE, 
        MPI_INT, 
        MPI_SUM, 
        MPI_COMM_WORLD
    );
  
    // end timing
    double end_time_builtin = MPI_Wtime(); 
    double time_builtin = end_time_builtin - start_time_builtin;

    message ="Built-in MPI_Allreduce"; 
    message += " Time: " + to_string(time_builtin) + " seconds | Result";

    print(globalArrayBuiltin,rank,message);

    // Fina;ize 
    MPI_Finalize();
    return 0;
}
