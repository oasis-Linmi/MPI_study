#include <iostream>
#include <stdio.h>
#include "mpi.h"


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

    for(int i = 2; i <= mysize + 1;i++){
        a[i][1] = 8.0;
        a[i][totalsize] = 8.0;
    }

    //Jacobi迭代部分
    for(int n = 1;n <= steps;n++){
        //从下侧的邻居得到数据

        if (myid < 3){
            MPI_Recv(&a[mysize+2][1], totalsize, MPI_DOUBLE_PRECISION, myid+1, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // printf("%f", a[mysize+2][2]);
            // printf("%f", a[mysize+2][totalsize-1]);
        }
        //向上侧的邻居发送数据
        if(myid > 0){
            MPI_Send(&a[2][1], totalsize, MPI_DOUBLE_PRECISION, myid-1, 11, MPI_COMM_WORLD);
        }

        //向下侧的邻居发送数据
        if(myid < 3){
            MPI_Send(&a[mysize+1][1], totalsize, MPI_DOUBLE_PRECISION, myid+1, 10, MPI_COMM_WORLD);
        }

        //从上侧的邻居得到数据
        if(myid > 0){
            MPI_Recv(&a[1][1], totalsize, MPI_DOUBLE_PRECISION, myid-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // printf("%f", a[1][totalsize]);
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