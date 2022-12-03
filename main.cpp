// send in number of processes and those processes will be partitioned into four groups(Blue, Yellow, Green, Red)
// mpic++ main.cpp -o main.out
// mpirun --hostfile mpi.config -np 4 ./main.out
#include <mpi.h>

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const* argv[]) {
    string colors[4] = {"Blue", "Yellow", "Green", "Red"};

    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int process_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    int color = process_rank / 4;
    MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, process_rank, &new_comm);

    int new_rank;
    MPI_Comm_rank(new_comm, &new_rank);

    int new_size;
    MPI_Comm_size(new_comm, &new_size);

    printf("Rank %d/%d in original comm, group [%s] in new comm\n", process_rank, world_size, colors[new_rank].c_str());

    MPI_Finalize();

    return 0;
}
