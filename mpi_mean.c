#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

#define filterSize 3    
//#define filterSzie 5
#define imageWidth 256
#define imageHight 256
//#define imageWidth 1024
//#define imageHight 1024
#define MASTER 0		//taskid of the first task
#define FROM_MASTER 1	//setting a message type
#define FROM_WORKER 2 	//setting a message type 

//imageHeight

int main(int argc, char *argv[])
{
	//define all variables here
	int	numtasks,              /* number of tasks in partition */
		taskid,                /* a task identifier */
		numworkers,            /* number of worker tasks */
		source,                /* task id of message source */
		dest,                  /* task id of message destination */
		mtype,                 /* message type */
		rows,                  /* rows of image sent to each worker */
		averow, extra, offset, /* used to determine rows sent to each worker */

		i, j, k, rc;           /* misc */
	int baseColumn;   //enable loop column shifting
	int baseRow;	  //enable loop row shifting
	double temp_pixel = 0;
	double image[imageWidth][imageHight];
	double result[imageWidth][imageHight];


	MPI_Status status;
	/* MPI programs start with MPI_Init; all 'N' processes exist thereafter*/
	MPI_Init(&argc, &argv);
	/* find out this processes' rank is*/
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	if (numtasks < 2) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}
	numworkers = numtasks - 1;

	/**************************************Master Task**************************************/
	if (taskid == MASTER)
	{
		printf("mpi_mean has started with %d tasks. \n", numtasks);
		printf("Initializing image...\n");
		for (i = 0; i < imageWidth; i++)
		{
			for (j = 0; i < imageHight; j++)
			{
				image[i][j] = rand() % 256;
			}
		}
		for (i = 0; i < imageWidth; i++)
		{
			for (j = 0; i < imageHight; j++)
			{
				result[i][j] = 0;
			}
		}
		printf("image initialization done!\n");
			//rand function

		/* Send image data to the worker tasks */
		averow = imageHight / numworkers;
		extra = imageHight % numworkers;
		offset = 0;
		mtype = FROM_MASTER;
		
		//printf("send image data to the worker tasks!\n");
		for (dest = 1; dest <= numworkers; dest++)
		{
			rows = (dest <= extra) ? averow + 1 : averow;
			printf("Sending %d rows to task %d offset = %d\n", rows, dest, offset); //print offset
			MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&image[offset][0], rows*imageWidth, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);

			offset = offset + rows;
		}

		/* Receive results from worker tasks*/
		
		//printf("Receive results from worker tasks!\n");
		mtype = FROM_WORKER;
		for (i = 1; i < numworkers; i++)
		{
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&result[offset][0], rows*imageWidth, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
			printf("Received results from task %d\n", source);   //printout 
		}

		/* Print Results if required */
	  printf("******************************************************\n");
      printf("Result Matrix:\n");
      for (i = 0; i < imageWidth; i++)
      {
         printf("\n"); 
         for (j = 0; j < imageHight; j++) 
            printf("%6.2f   ", result[i][j]);
      }
      printf("\n******************************************************\n");
      printf ("Done.\n");
	}


	/**************************************Worker Task**************************************/
	printf("Worker task initialization!\n");
	if (taskid > MASTER)
	{
		//printf("receive from master...\n");
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&image, rows*imageWidth, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

		//printf("Start filtering....\n");
		//filtering process 
		for (baseRow = 0; baseRow < rows - filterSize + 1; baseRow++)
		{
			for (baseColumn = 0; baseColumn < imageWidth - filterSize + 1; baseColumn++)
			{
				for (j = baseRow; j < filterSize + baseRow; j++)
				{
					for (k = baseColumn; k < filterSize + baseColumn; k++)
					{
						temp_pixel = image[j][k] + temp_pixel;
					}
				}
				image[j - 1][k - 1] = temp_pixel / 9;
				temp_pixel = 0;
			}

		}
		
		for (i = 0; i < imageWidth; i++)
		{
			for (j = 0; i < imageHight; j++)
			{
				result[i][j] = image[i][j];
			}
		}
		
		//printf("send to master...\n");
		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&result, rows*imageWidth, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}





