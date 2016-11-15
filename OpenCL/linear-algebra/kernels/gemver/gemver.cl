/**
 * gemver.cl: This file is part of the PolyBench/GPU 1.0 test suite.
 *
 *
 * Contact: Scott Grauer-Gray <sgrauerg@gmail.com>
 * Will Killian <killian@udel.edu>
 * Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://www.cse.ohio-state.edu/~pouchet/software/polybench/GPU
 */

#if defined(cl_khr_fp64)  // Khronos extension available?
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)  // AMD extension available?
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif

typedef float DATA_TYPE;


	
__kernel void gemver_kernel1(__global DATA_TYPE *A, __global DATA_TYPE *V1, __global DATA_TYPE *V2, __global DATA_TYPE *U1, __global DATA_TYPE *U2, int n, int cX, int cY) 
{    

	//int j = get_global_id(0);
	//int i = get_global_id(1);
	
	for (int y = 0; y < cY; y++) {
    	for (int x = 0; x < cX; x++) {

			int j = (get_group_id(0) * cX + x) * get_local_size(0) + get_local_id(0);
      		int i = (get_group_id(1) * cY + y) * get_local_size(1) + get_local_id(1);

			if ((i < n) && (j < n))
			{
				A[i*n + j] += U1[i] * V1[j] + U2[i] * V2[j];
			}
		}
	}
}


__kernel void gemver_kernel2(__global DATA_TYPE *A, __global DATA_TYPE *X, __global DATA_TYPE *Y, __global DATA_TYPE *Z, DATA_TYPE beta, int n, int cX, int cY) 
{    
	//int i = get_global_id(0);

	for (int x = 0; x < cX; x++) {
		int i = (get_group_id(0) * cX + x) * get_local_size(0) + get_local_id(0);

		if (i < n)
		{
			X[i] = 0;
			int j;
			for(j = 0; j < n; j++) 
			{
				X[i] += beta * A[j * n + i] * Y[j];
			}
			X[i] += Z[i];
		}
	}
}


__kernel void gemver_kernel3(__global DATA_TYPE *A, __global DATA_TYPE *X, __global DATA_TYPE *w, DATA_TYPE alpha, int n, int cX, int cY) 
{    
	//int i = get_global_id(0);
	
	for (int x = 0; x < cX; x++) {
		int i = (get_group_id(0) * cX + x) * get_local_size(0) + get_local_id(0);
		if (i < n)
		{
			w[i] = 0;
			int j;
			for(j = 0; j < n; j++)
			{ 
				w[i] += alpha * A[i*n + j] * X[j];
			}
		}
	}
}
