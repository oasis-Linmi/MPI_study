#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
int main( int argc, char *argv[] ){
    int rank, size;
    int chunk = 2;
    /*发送到一个进程的数据块的大小*/
    int i,j;
    int *sb;
    int *rb;
    int status, gstatus;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    sb = (int *)malloc(size*chunk*sizeof(int));/*申请发送缓冲区*/
    if ( !sb ) {
        perror( "can't allocate send buffer" );
        MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
    }
    rb = (int *)malloc(size*chunk*sizeof(int));/*申请接收缓冲区*/
    if ( !rb ) {
        perror( "can't allocate recv buffer");
        free(sb);
        MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
    }
    for ( i=0 ; i < size ; i++ ) {
        for ( j=0 ; j < chunk ; j++ ) {
            sb[i*chunk+j] = rank + i*chunk+j;/*设置发送缓冲区的数据*/
            printf("myid=%d,send to id=%d, data[%d]=%d\n",rank,i,j,sb[i*chunk+j]);
            rb[i*chunk+j] = 0;/*将接收缓冲区清0*/
        }
    }
    /* 执行MPI_Alltoall 调用*/
    MPI_Alltoall(sb,chunk,MPI_INT,rb,chunk,MPI_INT,MPI_COMM_WORLD);
    for ( i=0 ; i < size ; i++ ) {
        for ( j=0 ; j < chunk ; j++ ) {
            printf("myid=%d,recv from id=%d, data[%d]=%d\n",rank,i,j,rb[i*chunk+j]);
    /*打印接收缓冲区从其它进程接收的数据*/
        }
    }
    free(sb);
    free(rb);
    MPI_Finalize();
}
