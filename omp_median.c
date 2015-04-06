#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define filterSize 3    
//#define filterSzie 5
#define imageWidth 256
#define imageHight 256
//#define imageWidth 1024
//#define imageHight 1024

int main (int argc, char *argv[]) 
{
int	tid, nthreads, i, j, k, chunk;
int baseColumn;   //enable loop column shifting
int baseRow;	  //enable loop row shifting
int m, n;		//parameters for array sorting
double temp_pixel = 0;
double array[9] = {};
double image[imageWidth][imageHight];
double start_time, end_time;

chunk = 10;                    /* set loop iteration chunk size */

/*** Spawn a parallel region explicitly scoping all variables ***/
start_time = omp_get_wtime();
#pragma omp parallel shared(image,nthreads,chunk) private(tid,i,j,k)
  {
  tid = omp_get_thread_num();
  if (tid == 0)
    {
    nthreads = omp_get_num_threads();
    printf("Starting image filtering example with %d threads\n",nthreads);
    printf("Initializing image parameters...\n");
    }
  /*** Initialize image ***/
  #pragma omp for schedule (static, chunk) 
  	for (i = 0; i < imageHight; i++)
	{
		for (j = 0; j < imageWidth; j++)
		{
			image[i][j] = rand() % 256;
		}
	}	//rand function
  /*** Do image filtering sharing iterations on outer loop ***/
  /*** Display who does which iterations for demonstration purposes ***/
  printf("Thread %d starting image filtering...\n",tid);
  #pragma omp for schedule (static, chunk)

	for (baseRow = 0; baseRow < imageHight - filterSize + 1; baseRow++)		//repeat the loop by shifting
	{
		printf("Thread = %d did row = %d\n", tid, i);
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

			for (m = 0; m < 8; m++)								//sorting the array, find the median value is located in the array[(filterSize*filterSize-1)/2]
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

			//temp_pixel = array[4];	// got the median pixel
			image[i - 1][j - 1] = array[4];		//replace the heart pixel by the median value
			k = 0;
			temp_pixel = 0;
		}
	}
  }   /*** End of parallel region ***/
  end_time = omp_get_wtime();
  printf("Start time is: %f\tEnd time is: %f\tTotal elapsed Time is: %f\n", start_time, end_time, end_time - start_time);
/*** Print results if required***/


}