#include<stdio.h>
#include "../../../utilities/remapping.h"
#include "../../../utilities/remapping_mode.h"

// define vector length, stencil radius, 
#define N (GRIDSIZE * BLOCKSIZE * DATA_PER_THREAD)
#define DATA_PER_THREAD 3000 * 2 // 3000 is can fill up the 20MB cache
#define RADIUS 2000
#define GRIDSIZE 13
#define BLOCKSIZE 128

// -------------------------------------------------------
// CUDA device function that performs 1D stencil operation
// -------------------------------------------------------
__global__ void stencil_1D(int *in, int *out, int dim){

  //int lindex = threadIdx.x + RADIUS;
  //int tid = threadIdx.x + blockDim.x * blockIdx.x;
  int tid = remappingBlockIDx(blockIdx.x, BLOCKXMODE) * blockDim.x + remappingThreadIDx(threadIdx.x, THREADXMODE);
  //int stride = gridDim.x * blockDim.x;
  int result = 0;
  for (int j = -RADIUS ; j < DATA_PER_THREAD + RADIUS; j++){
  	result += in[tid * DATA_PER_THREAD + RADIUS + j];
  }
  out[tid] = result;
}

// ------------
// main program
// ------------
int main(void){

  //printf("start main\n");
  int *h_in = (int *) malloc(sizeof(int) * (N + 2 * RADIUS));
  int *h_out = (int *) malloc(sizeof(int) * (GRIDSIZE * BLOCKSIZE));
  int *d_in, *d_out;
  int size_in = (N + 2 * RADIUS) * sizeof(int);
  int size_out = (GRIDSIZE * BLOCKSIZE) * sizeof(int);
  int i, j, result, err;

  //printf("Start to alloc\n");
  cudaMalloc((void **)&d_in, size_in);
  cudaMalloc((void **)&d_out, size_out);

  //printf("Start to init\n");
  for (i=0; i< N + 2 * RADIUS; i++){
	h_in[i] = 1;
  }

  cudaMemcpy(d_in, h_in, size_in, cudaMemcpyHostToDevice);

  // Apply stencil by launching a sufficient number of blocks
  //printf("\n---------------------------\n");
  //printf("Launching 1D stencil kernel\n");
  //printf("---------------------------\n");
  //printf("Vector length	 = %d (%d MB)\n",N,N*4/1024/1024);
  //printf("Stencil radius	= %d\n",RADIUS);
  //printf("Blocks			= %d\n",GRIDSIZE);
  //printf("Threads per block = %d\n",BLOCKSIZE);
  //printf("Total threads	 = %d\n",GRIDSIZE*BLOCKSIZE);

  stencil_1D<<<GRIDSIZE,BLOCKSIZE>>>(d_in, d_out, N);

  // copy results back to host
  cudaMemcpy(h_out, d_out, size_out, cudaMemcpyDeviceToHost);

  // deallocate memory
  cudaFree(d_in);
  cudaFree(d_out);

  // check results
  err = 0;
  for (i=0; i< (GRIDSIZE * BLOCKSIZE) ; i++){
	result = 0;

	for (j = -RADIUS ; j < DATA_PER_THREAD + RADIUS; j++){
		result += h_in[i * DATA_PER_THREAD + RADIUS + j];
	}
	if (h_out[i] != result) {
	  err++;
	  printf("h_out[%d]=%d, cpu result = %d\n",i,h_out[i], result);
	}
  }

  if (err != 0){
	printf("\n Error, %d elements do not match!\n\n", err);
  } else {
	printf("\n Success! All elements match CPU result.\n\n");
  }

  return 0;

}
