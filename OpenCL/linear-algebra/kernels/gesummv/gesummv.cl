/**
 * gesummv.cl: This file is part of the PolyBench/GPU 1.0 test suite.
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


	
__kernel void gesummv_kernel(__global DATA_TYPE *a, __global DATA_TYPE *b, __global DATA_TYPE *x, __global DATA_TYPE *y, __global DATA_TYPE *tmp, DATA_TYPE alpha, DATA_TYPE beta, int n, int cX, int cY) 
{    
	
	for (int cx = 0; cx < cX; cx++) {

        int i = (get_group_id(0) * cX + cx) * get_local_size(0) + get_local_id(0);
		//int i = get_global_id(0);

		if (i < n)
		{
			tmp[i] = 0;
			y[i] = 0;
			int j;
			for(j = 0; j < n; j++)
			{	
				tmp[i] += a[i * n + j] * x[j];
				y[i] += b[i * n + j] * x[j];
			}
			y[i] = alpha * tmp[i] + beta * y[i];
		}
	}
}

