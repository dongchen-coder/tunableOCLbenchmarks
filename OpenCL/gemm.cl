/**
 * gemm.cl: This file is part of the PolyBench/GPU 1.0 test suite.
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


	
__kernel void gemm(__global DATA_TYPE *a, __global DATA_TYPE *b, __global DATA_TYPE *c, DATA_TYPE alpha, DATA_TYPE beta, int ni, int nj, int nk, int cX, int cY) 
{
  //int j = get_global_id(0);
	//int i = get_global_id(1);

	for (int y = 0; y < cY; y++) {
    for (int x = 0; x < cX; x++) {
			int j = (get_group_id(0) * cX + x) * get_local_size(0) + get_local_id(0);
      int i = (get_group_id(1) * cY + y) * get_local_size(1) + get_local_id(1);

			if ((i < ni) && (j < nj))
			{	
				c[i * nj + j] *= beta;
				int k;
				for(k=0; k < nk; k++)
				{
					c[i * nj + j] += alpha * a[i * nk + k] * b[k * nj +j];
				}
			}
		}
	}
}

