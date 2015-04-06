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

main(int argc, char *argv[])
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
	int m, n;		//parameters for array sorting
	int baseColumn;   //enable loop column shifting
	int baseRow;	  //enable loop row shifting
	double array[9] = {};
	double temp_pixel = 0;
	double image[imageWidth][imageHight];


	MPI_Status status;
	/* MPI programs start with MPI_Init; all 'N' processes exist thereafter*/
	MPI_Init(&argc, &argv);
	/* find out this processes' rank is*/
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	/* find out how big the SPMD world is*/
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	/* At this point, all programs are running equivalently, the rank distinguishes the roles of
	the roles of the programs in the SPMD model, with rank 0 often used specially...*/
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
			for (j = 0; j < imageHight; j++)
			{
				image[i][j] = rand() % 256;
			}
		}
		  //rand function
		//filterSize is already defined previously 

		/* Send image data to the worker tasks */
		averow = imageHight / numworkers;
		extra = imageHight % numworkers;
		offset = 0;
		mtype = FROM_MASTER;

		for (dest = 1; dest <= numworkers; dest++)
		{
			rows = (dest <= extra) ? averow + 1 : averow;
			printf("Sending %d rows to task %d offset = %d\n", rows, dest, offset);
			MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
			MPI_Send(&image[offset][0], rows*imageWidth, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);

			offset = offset + rows;
		}

		/* Receive results from worker tasks*/
		mtype = FROM_WORKER;
		for (i = 1; i < numworkers; i++)
		{
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&image[offset][0], rows*imageWidth, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
			printf("Received results from task %d\n", source);
		}

		/* Print Results if required */

	}


	/**************************************Worker Task**************************************/
	if (taskid > MASTER)
	{
		mtype = FROM_MASTER;
		MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
		MPI_Recv(&image, rows*imageWidth, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);


		//filtering process 
		for (baseRow = 0; baseRow < rows - filterSize + 1; baseRow++)		//repeat the loop by shifting
		{
			for (baseColumn = 0; baseColumn < imageWidth - filterSize + 1; baseColumn++)
			{
				for (i = baseRow; i < baseRow + filterSize; i++)			//let array store the 3*3 matrix
				{
					for (j = baseColumn; j < baseColumn + filterSize; j++)
					{
						array[k] = array[k] + image[i][j];
						k++;
					}
				}

				for (m = 0; m < 8; m++)								
				//sorting the array, find the median value is located in the array[(filterSize*filterSize-1)/2]
				{
					for (n = 0; n < 8; n++)
					{
						if (array[n] > array[n + 1])
						{
							temp_pixel = array[n];
							array[n] = array[n + 1];
							array[n + 1] = temp_pixel;
						}
					}
				}

				temp_pixel = array[4];	// got the median pixel
				image[i - 1][j - 1] = array[4];		//replace the heart pixel by the median value
				k = 0;
			}
		}

		mtype = FROM_WORKER;
		MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
		MPI_Send(&image, rows*imageWidth, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}





















