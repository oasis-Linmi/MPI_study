#include "mpi.h"
#include <stdio.h>
#include <math.h>
double f(double);
double f(double x)
/* 定义函数f(x) */
{
 return (4.0 / (1.0 + x*x));
}
int main(int argc,char *argv[])
{
    int done = 0, n, myid, numprocs, i;
    double PI25DT = 3.141592653589793238462643;
    /* 先给出已知的较为准确的p值*/
    double mypi, pi, h, sum, x;
    double startwtime = 0.0, endwtime;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    fprintf(stdout,"Process %d of %d on %s\n",
    myid, numprocs, processor_name);
    n = 0;
    if (myid == 0)
    { 
        printf("Please give N=");
        scanf("%d", &n);
        startwtime = MPI_Wtime();
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);/*将n值广播出去*/
    h = 1.0 / (double) n;/*得到矩形的宽度 所有矩形的宽度都相同*/
    sum = 0.0; /*给矩形面积赋初值*/
    for (i = myid + 1; i <= n; i += numprocs)
    /*每一个进程计算一部分矩形的面积 若进程总数numprocs为4 将0-1区间划分为100个
    矩形 则各个进程分别计算矩形块
    0进程 1 5 9 13 ... 97
    1进程 2 6 10 14 ... 98
    2进程 3 7 11 15 ... 99
    3进程 4 8 12 16 ... 100
    */
    {
        x = h * ((double)i - 0.5);
        sum += f(x);
    }
    mypi = h * sum;/*各个进程并行计算得到的部分和*/
    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0,
    MPI_COMM_WORLD);
    /*将部分和累加得到所有矩形的面积 该面积和即为近似p值*/
    if (myid == 0)
    /*执行累加的0号进程将近似值打印出来*/
    {
        printf("pi is approximately %.16f, Error is %.16f\n",
        pi, fabs(pi - PI25DT));
        endwtime = MPI_Wtime();
        printf("wall clock time = %f\n", endwtime-startwtime);
        fflush( stdout );
        }
    MPI_Finalize();
}