#include <iostream>
#include <stdio.h>
#include "mpi.h"
int main(int argc, char** argv ){
    int rank;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    int gsize, rbuf[100];
    int root, *sendbuf;
    root = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &gsize);
    sendbuf = (int *)malloc(gsize * 100 * sizeof(int));
    for(int i = 0;i < gsize * 100;i++){
        sendbuf[i] = i;
    }
    MPI_Scatter( sendbuf , 100 , MPI_INT , rbuf , 100 , MPI_INT , root , MPI_COMM_WORLD);

    if (rank != 0){
        std::cout << "Process " << rank << " receive:" << std::endl; 
        for(int i = 0;i < 100;i++){
            
            std::cout << rbuf[i] << " ";
        }
        std::cout << std::endl;
    }
    MPI_Finalize( );
    return 0;
}