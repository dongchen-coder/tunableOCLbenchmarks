/**
 * atax.cl: This file is part of the PolyBench/GPU 1.0 test suite.
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

__kernel void atax_kernel1(__global DATA_TYPE *A, __global DATA_TYPE *x, __global DATA_TYPE *tmp, int nx, int ny, int cX, int cY) {
    
	//int i = get_global_id(0);

	for (int cx = 0; cx < cX; cx++) {

		int i = (get_group_id(0) * cX + cx) * get_local_size(0) + get_local_id(0);
		if (i < nx)
		{
			tmp[i] = 0;
			int j;
			for(j=0; j < ny; j++)
			{
				tmp[i] += A[i * ny + j] * x[j];
			}
		}
	}
}

__kernel void atax_kernel2(__global DATA_TYPE *A, __global DATA_TYPE *y, __global DATA_TYPE *tmp, int nx, int ny, int cX, int cY) {
 	for (int cx = 0; cx < cX; cx++) {   
		//int j = get_global_id(0);

		int j = (get_group_id(0) * cX + cx) * get_local_size(0) + get_local_id(0);
		if (j < ny)
		{
			y[j] = 0;
			int i;
			for(i=0; i < nx; i++)
			{
				y[j] += A[i * ny + j] * tmp[i];
			}
		}
	}
}

