#include <iostream>
#include <stdio.h>
#include "mpi.h"
//发送和接受操作req之间彼此没有关系，发送和接受的进程都不同

int main (int argc, char **argv)
{
    int myid, numprocs;
    int totalsize = 16;
    //定义全局数组的规模
    int mysize = totalsize / 4;
    //迭代次数
    // std::cout << "Enter the times of Iteration:" << std::endl;
    // int steps;
    // std::cin >> steps;
    int steps = 10;
    int i, j, rc;
    int begin_row, end_row, ierr;
    // int status(MPI_F_STATUS_SIZE);

    //定义局部数组，第0行第0列不使用
    double a[mysize+3][totalsize+1], b[mysize+3][totalsize+1];

    //用于非阻塞同步的数据结构，req与sta以发送进程的编号作为索引
    MPI_Request req_send_to_up[4];
    MPI_Request req_send_to_down[4];
    MPI_Status sta_send_to_up[4];
    MPI_Status sta_send_to_down[4];

    MPI_Init (&argc, &argv);
    // MPI_Init(ierr);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    std::cout << "Process " << myid << " of " << numprocs << " is alive!" << std::endl;

    for(i = 1; i <= mysize + 2; i++){
        for(j = 1; j <= totalsize;j++){
            a[i][j] = 0.0;
        }
    }

    if (myid == 0){
        for(i = 1; i <= totalsize;i++){
            a[2][i] = 8.0;
        }
    }

    if (myid == 3){
        for(i = 1; i <= totalsize;i++){
            a[mysize+1][i] = 8.0;
        }
    }

    for(int i = 1; i <= mysize + 2;i++){
        a[i][1] = 8.0;
        a[i][totalsize] = 8.0;
    }

    //Jacobi迭代部分
    for(int n = 1;n <= steps;n++){
        //从下侧的邻居得到数据
        if (myid < 3){
            MPI_Irecv(&a[mysize+2][1], totalsize, MPI_DOUBLE_PRECISION, myid+1, 10, MPI_COMM_WORLD, &req_send_to_up[myid+1]);
            MPI_Wait(&req_send_to_up[myid+1], &sta_send_to_up[myid+1]);
        }
        //向上侧的邻居发送数据
        if(myid > 0){
            MPI_Isend(&a[2][1], totalsize, MPI_DOUBLE_PRECISION, myid-1, 10, MPI_COMM_WORLD, &req_send_to_up[myid]);
            MPI_Wait(&req_send_to_up[myid], &sta_send_to_up[myid]);
        }

        //向下侧的邻居发送数据
        if(myid < 3){
            MPI_Isend(&a[mysize+1][1], totalsize, MPI_DOUBLE_PRECISION, myid+1, 10, MPI_COMM_WORLD, &req_send_to_down[myid]);
            MPI_Wait(&req_send_to_down[myid], &sta_send_to_down[myid]);
        }

        //从上侧的邻居得到数据
        if(myid > 0){
            MPI_Irecv(&a[1][1], totalsize, MPI_DOUBLE_PRECISION, myid-1, 10, MPI_COMM_WORLD, &req_send_to_down[myid-1]);
            MPI_Wait(&req_send_to_down[myid-1], &sta_send_to_down[myid-1]);
        }

        begin_row = 2;
        end_row = mysize + 1;
        if(myid == 0){
            begin_row = 3;
        }
        if(myid == 3){
            end_row = mysize;
        }
        //计算过程
        for(i = begin_row; i <= end_row; i++){
            for(j = 2; j <= totalsize - 1; j++){
                b[i][j] = (a[i][j+1] + a[i][j-1] + a[i-1][j] + a[i+1][j]) * 0.25;
            }
        }
        //拷贝回数组a
        for(i = begin_row; i <= end_row; i++){
            for(j = 2; j <= totalsize - 1; j++){
                a[i][j] = b[i][j];
            }
        }
    }

    std::cout << "After jacobi iteration of process " << myid << ", the partial matrix is:" << std::endl; 
    for(i = 2;i <= mysize + 1;i++){
        for(j = 1;j <= totalsize;j++){
            printf("%8.2f",a[i][j]);
        }
        std::cout << std::endl;
    }
    MPI_Finalize ();
    return 0;
}