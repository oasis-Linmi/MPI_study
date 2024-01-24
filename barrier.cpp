#include "mpi.h"
// #include "test.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
int main( int argc, char **argv )
{
    int rank, size, i;
    int *table;
    int errors=0;
    MPI_Aint address;
    MPI_Datatype type, newtype;
    int lens;
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    /* Make data table */
    table = (int *) calloc (size, sizeof(int));
    table[rank] = rank + 1; /*准备要广播的数据*/
    MPI_Barrier ( MPI_COMM_WORLD );
    /* 将数据广播出去*/
    for ( i=0; i<size; i++ )
        //第i个数组元素，第i个进程是root进程...
        MPI_Bcast( &table[i], 1, MPI_INT, i, MPI_COMM_WORLD );
    /* 检查接收到的数据的正确性 */
    for ( i=0; i<size; i++ )
        if (table[i] != i+1) errors++;
            MPI_Barrier ( MPI_COMM_WORLD );/*检查完毕后执行一次同步*/
    std::cout << "The process " << rank <<" has "<< errors << " errors!" <<  std::endl;
    /*其它的计算*/
    MPI_Finalize();
}