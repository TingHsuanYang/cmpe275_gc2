// send in number of processes and those processes will be partitioned into four groups(Blue, Yellow, Green, Red)
// mpic++ main.cpp -o main.out
// mpirun --hostfile mpi.config -np 4 ./main.out test.txt out.txt
#include <mpi.h>
#include <stdio.h>

#include <iostream>
#include <string>

using namespace std;

/* remove the unwanted character*/
void removePunctuation(char* chunk) {
    // move forward the character if previous character is non-alphabet
    int count = 0;
    int i = 0, j = 0;
    for (i = 0; chunk[i] != '\0'; i++) {
        if ((chunk[i] >= 'a' && chunk[i] <= 'z') || (chunk[i] >= 'A' && chunk[i] <= 'Z')) {
            chunk[j] = chunk[i];
            j++;
        }
    }

    // add null character at the end of the string
    chunk[j] = '\0';
}

/* convert the string to lower case*/
void convertToLower(char* chunk) {
    int i = 0;
    while (chunk[i] != '\0') {
        if (chunk[i] >= 'A' && chunk[i] <= 'Z') {
            chunk[i] = chunk[i] + 32;
        }
        i++;
    }
}

int calculateFrequency(char* chunk, int* freq) {
    // int freq[26] = {0};
    int i = 0;
    while (chunk[i] != '\0') {
        if (chunk[i] >= 'a' && chunk[i] <= 'z') {
            freq[chunk[i] - 'a']++;
        }
        i++;
    }

    return *freq;
}

void create_Intercommunicator(MPI_Comm& comm, int color, MPI_Comm& a, MPI_Comm& b, MPI_Comm& c, MPI_Comm& d) {
    if (color == 0) {  // Blue: remove the unwanted word in text file

        /* Creates an intercommunicator from two intracommunicators. */
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 1, 1, &a);
    } else if (color == 1) {  // Yellow
        // set to one word each line
        //     setOneCharacter(&in, &out, group_rank, group_size, overlap);
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 0, 1, &a);
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 2, 12, &b);
    } else if (color == 2) {  // Green
        // count the word frequency
        //     countFrequency(&in, &out, group_rank, group_size, overlap);
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 1, 12, &b);
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 3, 123, &c);
    } else if (color == 3) {  // Red
        // sort by the word frequency
        //     sortByFrequency(&in, &out, group_rank, group_size, overlap);
        MPI_Intercomm_create(comm, 0, MPI_COMM_WORLD, 2, 123, &c);
    }
}

int main(int argc, char const* argv[]) {
    string colors[4] = {"Blue", "Yellow", "Green", "Red"};
    MPI_Comm group_comm, BY_comm, YG_comm, GR_comm, RD_comm;
    MPI_File in, out;
    MPI_Offset filesize; /*  integer type of size sufficient to represent the size (in bytes) */
    MPI_Status status;   /* Status returned from read */
    int world_rank, world_size, group_rank, group_size;
    int initialized, finalized;
    int ierr;
    int bufsize;
    int freqsum[26] = {0};

    MPI_Initialized(&initialized);
    if (!initialized)
        MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /* Check the arguments */
    if (argc != 3) {
        if (world_rank == 0)
            fprintf(stderr, "Usage: %s infilename outfilename\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    /* Read the input file */
    ierr = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &in);
    if (ierr) {
        if (world_rank == 0)
            fprintf(stderr, "%s: Couldn't open file %s\n", argv[0], argv[1]);
        MPI_Finalize();
        exit(2);
    }

    /* Get the size of the file */
    ierr = MPI_File_get_size(in, &filesize);
    if (ierr) {
        if (world_rank == 0)
            fprintf(stderr, "%s: Couldn't read file size of %s\n", argv[0], argv[1]);
        MPI_Finalize();
        exit(3);
    }

    /* Open the output file */
    ierr = MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out);
    if (ierr) {
        if (world_rank == 0)
            fprintf(stderr, "%s: Couldn't open output file %s\n", argv[0], argv[2]);
        MPI_Finalize();
        exit(4);
    }

    /* split into groups*/
    int color = world_rank % 4;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &group_comm);
    MPI_Comm_rank(group_comm, &group_rank);
    MPI_Comm_size(group_comm, &group_size);

    printf("Rank %d/%d in original comm, group [%s] %d/%d in new comm\n", world_rank, world_size, colors[color].c_str(), group_rank, group_size);

    /* Calculate how many elements that is */
    filesize = filesize / sizeof(char);
    /* Calculate how many elements each processor gets */
    bufsize = filesize / group_size;

    ierr = MPI_File_set_view(in, group_rank * bufsize, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);  // split the file for group members
    if (ierr) {
        if (group_rank == 0)
            fprintf(stderr, "%s: Couldn't set file view for %s", argv[0], argv[1]);
        MPI_Finalize();
        exit(5);
    }

    ierr = MPI_File_set_view(out, group_rank * bufsize, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);  // split the file for group members
    if (ierr) {
        if (group_rank == 0)
            fprintf(stderr, "%s: Couldn't set file view for %s", argv[0], argv[1]);
        MPI_Finalize();
        exit(6);
    }

    /* create intercommunicator for groups */
    create_Intercommunicator(group_comm, color, BY_comm, YG_comm, GR_comm, RD_comm);

    MPI_Barrier(MPI_COMM_WORLD);

    if (color == 0) {  // Blue: remove the puctuations and spaces in text file
        int nrchar = 0;
        char* buf = new char[bufsize + 1];
        /* Reads a file starting at the location specified by the individual file pointer (blocking, noncollective) */
        ierr = MPI_File_read(in, buf, bufsize, MPI_CHAR, &status);
        if (ierr) {
            if (group_rank == 0)
                fprintf(stderr, "%s: Couldn't read from file %s", argv[0], argv[1]);
            MPI_Finalize();
            exit(7);
        }
        /* Gets the number of top-level elements received. */
        MPI_Get_count(&status, MPI_CHAR, &nrchar);
        /* Add a null character to the end of the buffer */
        buf[nrchar] = '\0';
        printf("\n------------------------[Blue]Process %d read %d characters--------------------\n%s\n", group_rank, nrchar, buf);
        removePunctuation(buf);
        printf("\n------------------------[Blue]Process %d After Process--------------------\n%s\n", group_rank, buf);

        MPI_Send(buf, bufsize, MPI_CHAR, group_rank, 0, BY_comm);
    } else if (color == 1) {  // Yellow: convert to lowercase
        char* buf = new char[bufsize + 1];
        /* receive the data from blue */
        MPI_Recv(buf, bufsize, MPI_CHAR, group_rank, 0, BY_comm, &status);
        printf("\n------------------------[Yellow]Process %d received %d characters--------------------\n", group_rank, bufsize);
        convertToLower(buf);
        printf("\n------------------------[Yellow]Process %d After Process--------------------\n%s\n", group_rank, buf);
        MPI_Send(buf, bufsize, MPI_CHAR, group_rank, 0, YG_comm);
    } else if (color == 2) {  // Green: calculate the char frequency
        char* buf = new char[bufsize + 1];
        int freq[26] = {0};
        /* receive the data from yellow */
        MPI_Recv(buf, bufsize, MPI_CHAR, group_rank, 0, YG_comm, &status);
        printf("\n------------------------[Green]Process %d received %d characters--------------------\n", group_rank, bufsize);
        calculateFrequency(buf, freq);
        printf("\n------------------------[Green]Process %d After Process--------------------\n", group_rank);
        for (int i = 0; i < 26; i++) {
            printf("%c: %d\n", 'a' + i, freq[i]);
        }
        MPI_Send(freq, 26, MPI_INT, group_rank, 0, GR_comm);
    } else if (color == 3) {  // Red: write the result to file
        int freq[26] = {0};
        MPI_Recv(freq, 26, MPI_INT, group_rank, 0, GR_comm, &status);
        printf("\n------------------------[Red]Process %d received %d characters--------------------\n", group_rank, bufsize);
        for (int i = 0; i < 26; i++) {
            printf("%c: %d\n", 'a' + i, freq[i]);
        }

        MPI_Reduce(freq, freqsum, 26, MPI_INT, MPI_SUM, 0, group_comm);

        if (group_rank == 0) {
            // print out freqsum
            printf("-----freqsum------\n");
            for (int i = 0; i < 26; i++) {
                printf("%c: %d\n", 'a' + i, freqsum[i]);
            }

            // write out to the file
            char outbuf[1000];
            int index = 0;
            for (int i = 0; i < 26; i++) {
                index += sprintf(outbuf + index, "%c: %d\n", 'a' + i, freqsum[i]);
            }

            ierr = MPI_File_write(out, outbuf, index, MPI_CHAR, &status);
            if (ierr) {
                if (group_rank == 0) fprintf(stderr, "%s: Couldn't write to file %s", argv[0], argv[2]);
                MPI_Finalize();
                exit(8);
            }
        }
    }

    MPI_File_close(&in);
    MPI_File_close(&out);

    MPI_Finalized(&finalized);
    if (!finalized)
        MPI_Finalize();

    return 0;
}
