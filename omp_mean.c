#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define filterSize 3    
//#define filterSize 5
#define imageWidth 256
#define imageHight 256
//#define imageWidth 1024
//#define imageHight 1024

int main (int argc, char *argv[]) 
{
int	tid, nthreads, i, j, k, chunk;
int baseColumn;   //enable loop column shifting
int baseRow;	  //enable loop row shifting
double temp_pixel = 0;
static double image[imageWidth][imageHight];
double start_time, end_time;


chunk = 10;                    /* set loop iteration chunk size */
num_threads = 4;
OMP_SET_NUM_THREADS(num_threads);
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

	for (baseRow = 0; baseRow < imageHight - filterSize + 1; baseRow++)
	{
		printf("Thread = %d did row = %d\n", tid ,baseRow);
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
  }   /*** End of parallel region ***/
  
  end_time = omp_get_wtime();
  printf("Start time is: %f\tEnd time is: %f\tTotal elapsed Time is: %f\n", start_time, end_time, end_time - start_time);

/*** Print results if required ***/


}