#include <iostream>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv){
    int MAX_ROWS, MAX_COLS,rows,cols;
    MAX_ROWS = 1000,MAX_COLS=1000;
    double a[MAX_ROWS][MAX_COLS], b[MAX_COLS], c[MAX_COLS];
    double buffer[MAX_COLS], ans;
    int myid, master, numprocs;
    int i, j, numsent, numrcvd, sender;
    int anstype, row;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    MPI_Status status;

    master = 0;
    rows = 100;
    cols = 100;

    //主进程操作
    if (myid == master){
        for(i = 1;i <= rows;i++){
            b[i] = 1;
            for(j = 1;j <= cols;j++){
                a[i][j] = i;
            }
        }
        numsent = 0;
        numrcvd = 0;
        MPI_Bcast(&b[1], cols, MPI_DOUBLE_PRECISION, master, MPI_COMM_WORLD);
        int min_num = std::min(numprocs-1, rows);
        for(i = 1;i <= min_num;i++ ){
            for(j = 1; j <= cols;j++){
                buffer[j] = a[i][j];
            }
            MPI_Send(&buffer[1], cols, MPI_DOUBLE_PRECISION, i, i, MPI_COMM_WORLD);
            numsent++;
        }
        
        //对所有的行，依次接受从进程对一行数据的计算结果
        for(i = 1;i <= rows;i++){
            MPI_Recv(&ans , 1 , MPI_DOUBLE_PRECISION , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &status);
            sender = status.MPI_SOURCE;
            anstype = status.MPI_TAG;
            c[anstype] = ans;
            if(numsent < rows){
                for(j = 1;j <= cols;j++){
                    buffer[j] = a[numsent+1][j];
                }
                MPI_Send(&buffer[1], cols, MPI_DOUBLE_PRECISION, sender, numsent+1, MPI_COMM_WORLD);
                numsent++;
            }else{
                MPI_Send(&buffer[1], 0, MPI_DOUBLE_PRECISION, sender, 0, MPI_COMM_WORLD);
                
            }
        }
        for(i = 1;i <= cols;i++){
            std::cout << c[i] << " ";
        }
    //从进程操作
    }else{
        MPI_Bcast(&b[1], cols, MPI_DOUBLE_PRECISION, master, MPI_COMM_WORLD);
        MPI_Recv(&buffer[1], cols, MPI_DOUBLE_PRECISION, master, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
        
        while (status.MPI_TAG != 0){
            row = status.MPI_TAG;
            ans = 0.0;
            for(i = 1;i <= cols;i++){
                ans += buffer[i] * b[i];
            }
            MPI_Send(&ans, 1, MPI_DOUBLE_PRECISION, master, row, MPI_COMM_WORLD);
            MPI_Recv(&buffer[1], cols, MPI_DOUBLE_PRECISION, master, MPI_ANY_TAG,MPI_COMM_WORLD, &status);
        }

    }
    MPI_Finalize();


    return 0;
}