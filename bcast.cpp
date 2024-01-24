#include <stdio.h>
#include "mpi.h"
int main(int argc, char** argv ){
    int rank, value;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    do {
    if (rank == 0) /*进程0读入需要广播的数据*/
    scanf( "%d", &value );
    MPI_Bcast( &value, 1, MPI_INT, 0, MPI_COMM_WORLD );/*将该数据广播出去*/
    printf( "Process %d got %d\n", rank, value );/*各进程打印收到的数据*/
    } while (value >= 0);
    MPI_Finalize( );
    return 0;
}