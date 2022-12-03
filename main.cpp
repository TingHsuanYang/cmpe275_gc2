// send in number of processes and those processes will be partitioned into four groups(Blue, Yellow, Green, Red)
// mpic++ main.cpp -o main.out
// mpirun --hostfile mpi.config -np 4 ./main.out foo.in foo.out
#include <mpi.h>

#include <iostream>
#include <string>

using namespace std;

void removeWord(MPI_File* in, MPI_File* out, const int rank, const int size, const int overlap) {
    MPI_Offset globalstart;
}

void setOneWord(MPI_File* in, MPI_File* out, const int rank, const int size, const int overlap) {
}

void countWordFrequency(MPI_File* in, MPI_File* out, const int rank, const int size, const int overlap) {
}

void sortByWordFrequency(MPI_File* in, MPI_File* out, const int rank, const int size, const int overlap) {
}

void parprocess(MPI_File* in, MPI_File* out, const int rank, const int size, const int overlap) {
    MPI_Offset globalstart;
    int mysize;
    char* chunk;

    /* read in relevant chunk of file into "chunk",
     * which starts at location in the file globalstart
     * and has size mysize
     */
    {
        MPI_Offset globalend;
        MPI_Offset filesize;

        /* figure out who reads what */
        MPI_File_get_size(*in, &filesize);
        filesize--; /* get rid of text file eof */
        mysize = filesize / size;
        globalstart = rank * mysize;
        globalend = globalstart + mysize - 1;
        if (rank == size - 1) globalend = filesize - 1;

        /* add overlap to the end of everyone's chunk except last proc... */
        if (rank != size - 1)
            globalend += overlap;

        mysize = globalend - globalstart + 1;

        /* allocate memory */
        chunk = new char[mysize + 1];
        // chunk = malloc((mysize + 1) * sizeof(char));

        /* everyone reads in their part */
        MPI_File_read_at_all(*in, globalstart, chunk, mysize, MPI_CHAR, MPI_STATUS_IGNORE);
        chunk[mysize] = '\0';
    }

    /*
     * everyone calculate what their start and end *really* are by going
     * from the first newline after start to the first newline after the
     * overlap region starts (eg, after end - overlap + 1)
     */

    int locstart = 0, locend = mysize - 1;
    if (rank != 0) {
        while (chunk[locstart] != '\n') locstart++;
        locstart++;
    }
    if (rank != size - 1) {
        locend -= overlap;
        while (chunk[locend] != '\n') locend++;
    }
    mysize = locend - locstart + 1;

    /* "Process" our chunk by replacing non-space characters with '1' for
     * rank 1, '2' for rank 2, etc...
     */

    for (int i = locstart; i <= locend; i++) {
        char c = chunk[i];
        chunk[i] = (isspace(c) ? c : '1' + (char)rank);
    }

    /* output the processed file */

    MPI_File_write_at_all(*out, (MPI_Offset)(globalstart + (MPI_Offset)locstart), &(chunk[locstart]), mysize, MPI_CHAR, MPI_STATUS_IGNORE);

    return;
}

int main(int argc, char const* argv[]) {
    string colors[4] = {"Blue", "Yellow", "Green", "Red"};
    MPI_File in, out;
    int rank, size;
    int ierr;
    const int overlap = 100;

    MPI_Init(&argc, (char***)&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // if (argc != 3) {
    //     if (rank == 0) fprintf(stderr, "Usage: %s infilename outfilename\n", argv[0]);
    //     MPI_Finalize();
    //     exit(1);
    // }

    // ierr = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &in);
    // if (ierr) {
    //     if (rank == 0) fprintf(stderr, "%s: Couldn't open file %s\n", argv[0], argv[1]);
    //     MPI_Finalize();
    //     exit(2);
    // }

    // ierr = MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    // if (ierr) {
    //     if (rank == 0) fprintf(stderr, "%s: Couldn't open output file %s\n", argv[0], argv[2]);
    //     MPI_Finalize();
    //     exit(3);
    // }

    int color = rank % 4;
    MPI_Comm group_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &group_comm);

    int group_rank;
    MPI_Comm_rank(group_comm, &group_rank);

    int group_size;
    MPI_Comm_size(group_comm, &group_size);

    printf("Rank %d/%d in original comm, group [%s] %d/%d in new comm\n", rank, size, colors[color].c_str(), color, group_size);

    if (color == 0) {  // Blue
        // remove the unwanted word in text file
        //     removeWord(&in, &out, group_rank, group_size, overlap);
        // } else if (color == 1) {  // Yellow
        //     // set to one word each line
        //     setOneWord(&in, &out, group_rank, group_size, overlap);
        // } else if (color == 2) {  // Green
        //     // count the word frequency
        //     countWordFrequency(&in, &out, group_rank, group_size, overlap);
        // } else if (color == 3) {  // Red
        //     // sort by the word frequency
        //     sortByWordFrequency(&in, &out, group_rank, group_size, overlap);
    }

    // MPI_File_close(&in);
    // MPI_File_close(&out);

    MPI_Finalize();
    return 0;
}
