#include <iostream>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv){
    std::cout << "Enter four integers: ROW partition number, COLUMN partition number, the size of matrix, the iteration times" << std::endl;
    int ROW_PARTITION = atoi(argv[1]);
    int COLUMN_PARTITION = atoi(argv[2]);
    int totalsize = atoi(argv[3]);
    int steps = atoi(argv[4]);
    int mysize_ROW = totalsize / ROW_PARTITION;
    int mysize_COLUMN = totalsize / COLUMN_PARTITION;
    double a[totalsize][totalsize];
    int myid, numprocs;
    double local_start, local_finish, local_elapsed, elapsed;
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    std::cout << "Process " << myid << " of " << numprocs << " is alive!" << std::endl;

    //Recording the time of parallel.
    MPI_Barrier( MPI_COMM_WORLD);
    local_start = MPI_Wtime();

    //Due to the two-dimention, we need to compute the small matrix index(a, b)
    int my_row_positon = myid / COLUMN_PARTITION;
    int my_column_position = myid % COLUMN_PARTITION;
    if(myid == 0){

        //initial
        for(int i = 0;i < totalsize;i++){
            for(int j = 0;j < totalsize;j++){
                if(i == 0 || j == 0 || i == totalsize-1 || j == totalsize-1){
                    a[i][j] = 8;
                }else{
                    a[i][j] = 0;
                }
            }
        }
   
    }
    //Compress each small matrix into a row in order
    double scatter_buffer[totalsize*totalsize];
    int small_size = totalsize * totalsize / ROW_PARTITION / COLUMN_PARTITION;
    for(int i = 0;i < ROW_PARTITION;i++){
        for(int j = 0;j < COLUMN_PARTITION;j++){
            int first_row = i * mysize_ROW;
            int first_column = j * mysize_COLUMN;
            for(int x = 0;x < mysize_ROW;x++){
                for(int y = 0;y < mysize_COLUMN;y++){
                    int seq = i * COLUMN_PARTITION + j;
                    scatter_buffer[seq*small_size+x*mysize_COLUMN+y] = a[x+first_row][y+first_column];
                }
            }
         }
    } 
   //Scatter receive buffer
    double receive_buffer[small_size];
    MPI_Scatter( scatter_buffer , small_size , MPI_DOUBLE_PRECISION , receive_buffer , small_size , MPI_DOUBLE_PRECISION , 0 , MPI_COMM_WORLD);
    
    // Build a small two dimension matrix,the first and last row (column) 
    // is used to store message received from other process.
    double b[mysize_ROW+2][mysize_COLUMN+2];
    double c[mysize_ROW+2][mysize_COLUMN+2];
    for(int i = 0;i < mysize_ROW;i++){
        for(int j = 0;j < mysize_COLUMN;j++){
            //The first row and column is not used to store valid data!
            b[i+1][j+1] = receive_buffer[i*mysize_COLUMN+j];
        }
    }

    int tag_left2right = 3;
    int tag_right2left = 4;
    int tag_up2down = 5;
    int tag_down2up =6;
    int left, right, up, down;
    if (my_column_position > 0){
        left = myid - 1;
    }else{
        left = MPI_PROC_NULL;
    }
    if (my_column_position < COLUMN_PARTITION - 1){
        right = myid + 1;
    }else{
        right = MPI_PROC_NULL;
    }
    if(my_row_positon > 0){
        up = myid - COLUMN_PARTITION;
    }else{
        up = MPI_PROC_NULL;
    }
    if(my_row_positon < ROW_PARTITION - 1){
        down = myid + COLUMN_PARTITION;
    }else{
        down = MPI_PROC_NULL;
    }
    MPI_Request req_array[8];
    MPI_Status sta_array[8];
    for(int n = 1;n <= steps;n++){
        
        //from up to down
        // MPI_Sendrecv(&b[mysize_ROW][1], mysize_COLUMN, MPI_DOUBLE_PRECISION, down,tag_up2down,
        //             &b[0][1],mysize_COLUMN, MPI_DOUBLE_PRECISION, up, tag_up2down, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Isend( &b[mysize_ROW][1] , mysize_COLUMN , MPI_DOUBLE_PRECISION , down , tag_up2down , MPI_COMM_WORLD , &req_array[0]);
        MPI_Irecv( &b[0][1],mysize_COLUMN, MPI_DOUBLE_PRECISION, up, tag_up2down, MPI_COMM_WORLD , &req_array[1]);
        MPI_Wait(&req_array[1], &sta_array[1]);
        //from down to up
        // MPI_Sendrecv(&b[1][1], mysize_COLUMN, MPI_DOUBLE_PRECISION, up, tag_down2up,
        //             &b[mysize_ROW+1][1], mysize_COLUMN,MPI_DOUBLE_PRECISION,down, tag_down2up, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Isend(&b[1][1], mysize_COLUMN, MPI_DOUBLE_PRECISION, up, tag_down2up, MPI_COMM_WORLD , &req_array[2]);
        MPI_Irecv(&b[mysize_ROW+1][1], mysize_COLUMN,MPI_DOUBLE_PRECISION,down, tag_down2up, MPI_COMM_WORLD, &req_array[3]);
        MPI_Wait(&req_array[3], &sta_array[3]);
        //from left to right(need a buffer to store a column)
        double sbuffer_lr_col[mysize_ROW];
        double rbuffer_lr_col[mysize_ROW];
        //construct the send buffer
        if(right != MPI_PROC_NULL){
            for(int i = 1; i <= mysize_ROW;i++){
                //the second column of b from right on
                sbuffer_lr_col[i-1] = b[i][mysize_COLUMN];
            }
        }
        // MPI_Sendrecv(sbuffer_lr_col, mysize_ROW, MPI_DOUBLE_PRECISION, right, tag_left2right,
        //             rbuffer_lr_col, mysize_ROW,MPI_DOUBLE_PRECISION, left, tag_left2right, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Isend(sbuffer_lr_col, mysize_ROW, MPI_DOUBLE_PRECISION, right, tag_left2right, MPI_COMM_WORLD , &req_array[4]);
        MPI_Irecv(rbuffer_lr_col, mysize_ROW,MPI_DOUBLE_PRECISION, left, tag_left2right, MPI_COMM_WORLD , &req_array[5]);
        MPI_Wait(&req_array[5], &sta_array[5]);
        //construct the b's first column using rbuffer.
        if(left != MPI_PROC_NULL){
            for(int i = 1;i <= mysize_ROW;i++){
                b[i][0] = rbuffer_lr_col[i-1];
            }
        }
        //from right to left(need a buffer to store a column)
        double sbuffer_rl_col[mysize_ROW];
        double rbuffer_rl_col[mysize_ROW];
        //construct the send buffer
        if(left != MPI_PROC_NULL){
            for(int i = 1; i <= mysize_ROW;i++){
                //the second column of b from left on
                sbuffer_rl_col[i-1] = b[i][1];
            }
        }
        // MPI_Sendrecv(sbuffer_rl_col, mysize_ROW, MPI_DOUBLE_PRECISION, left, tag_right2left,
        //             rbuffer_rl_col, mysize_ROW,MPI_DOUBLE_PRECISION, right, tag_right2left, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Isend(sbuffer_rl_col, mysize_ROW, MPI_DOUBLE_PRECISION, left, tag_right2left, MPI_COMM_WORLD , &req_array[6]);
        MPI_Irecv(rbuffer_rl_col, mysize_ROW,MPI_DOUBLE_PRECISION, right, tag_right2left, MPI_COMM_WORLD , &req_array[7]);
        MPI_Wait(&req_array[7], &sta_array[7]);
        //construct the b's first column using rbuffer.
        if(right != MPI_PROC_NULL){
            for(int i = 1;i <= mysize_ROW;i++){
                b[i][mysize_COLUMN+1] = rbuffer_rl_col[i-1];
            }
        }


        int begin_row, end_row, begin_column, end_column;
        begin_row = 1;
        end_row = mysize_ROW;
        begin_column = 1;
        end_column = mysize_COLUMN;

        if (my_column_position == 0){
            begin_column = 2;
        }
        if (my_column_position == COLUMN_PARTITION - 1){
            end_column = mysize_COLUMN - 1;
        }
        if(my_row_positon == 0){
            begin_row = 2;
        }
        if(my_row_positon == ROW_PARTITION - 1){
            end_row = mysize_ROW - 1;
        }

        //compute procession
        for(int i = begin_row; i <= end_row; i++){
            for(int j = begin_column; j <= end_column; j++){
                c[i][j] = (b[i][j+1] + b[i][j-1] + b[i-1][j] + b[i+1][j]) * 0.25;
            }
        }
        //copy to array b
        for(int i = begin_row; i <= end_row;i++){
            for(int j = begin_column; j <= end_column;j++){
                b[i][j] = c[i][j];
            }
        }

    }

    std::cout << "After jacobi iteration of process " << myid << ", the partial matrix is:" << std::endl; 
    for(int i = 1;i <= mysize_ROW;i++){
        for(int j = 1;j <= mysize_COLUMN;j++){
            printf("%8.2f",b[i][j]);
        }
        std::cout << std::endl;
    }
    double sum, maxsum;
    sum = 0.0;
    for(int i = 1;i <= mysize_ROW;i++){
        for(int j = 1;j <= mysize_COLUMN;j++){
            sum += b[i][j];
        }
    }
    MPI_Reduce( &sum , &maxsum , 1 , MPI_DOUBLE_PRECISION , MPI_MAX , 0 , MPI_COMM_WORLD);
    if(myid == 0){
        std::cout << "The max sum of all small matrix is: " << maxsum << std::endl;
    }
    //construct the gather construction
    double gather_send_buffer[small_size];
    for(int i = 1;i <= mysize_ROW;i++){
        for(int j = 1;j <= mysize_COLUMN;j++){
            gather_send_buffer[(i-1)*mysize_COLUMN+(j-1)] = b[i][j];
        }
    }
    double gather_receive_buffer[totalsize*totalsize];

    MPI_Gather( gather_send_buffer , small_size , MPI_DOUBLE_PRECISION , gather_receive_buffer , small_size , MPI_DOUBLE_PRECISION , 0 , MPI_COMM_WORLD);
    //use the gather_receive_buffer to construct the overall large matrix
    if(myid == 0){
        for(int i = 0;i < ROW_PARTITION;i++){
            for(int j = 0;j < COLUMN_PARTITION;j++){
                int first_row = i * mysize_ROW;
                int first_column = j * mysize_COLUMN;
                for(int x = 0;x < mysize_ROW;x++){
                    for(int y = 0;y < mysize_COLUMN;y++){
                        int seq = i * COLUMN_PARTITION + j;
                        a[x+first_row][y+first_column] = gather_receive_buffer[seq*small_size+x*mysize_COLUMN+y];
                    }
                }
            }
        }
        for(int i = 0;i < totalsize;i++){
            std::cout << std::endl;
            for(int j = 0;j < totalsize;j++){
                printf("%5.2f",a[i][j]);
            }
        }
        std::cout << std::endl;
    }
    local_finish = MPI_Wtime();
    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed,&elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    if(myid == 0){
        printf("Elapsed time = %e seconds\n",elapsed);
    }
    MPI_Finalize();
}