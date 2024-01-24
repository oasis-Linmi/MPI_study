#include <iostream>
#include <stdio.h>
#include "mpi.h"
int main(int argc, char** argv ){
    int rank;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    int gsize, sendarray[100];
    int root, *rbuf;
    root = 0;
    for(int i = 0;i < 100;i++){
        sendarray[i] = rank;
    }
    MPI_Comm_size(MPI_COMM_WORLD, &gsize);
    rbuf = (int *)malloc(gsize * 100 * sizeof(int));
    MPI_Gather( sendarray , 100 , MPI_INT , rbuf , 100 , MPI_INT , root , MPI_COMM_WORLD);
    if (rank == 0){
        for(int i = 0;i < gsize * 100;i++){
            std::cout << rbuf[i] << " ";
        }

    }
    MPI_Finalize( );
    return 0;
}