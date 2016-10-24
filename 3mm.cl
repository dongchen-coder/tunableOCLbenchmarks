/**
 * 3mm.cl: This file is part of the PolyBench/GPU 1.0 test suite.
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

__kernel void mm3_kernel1(__global DATA_TYPE *A, __global DATA_TYPE *B, __global DATA_TYPE *E, int ni, int nj, int nk, int cX, int cY) 
{    

	for (int y = 0; y < cY; y++) {
    for (int x = 0; x < cX; x++) {
			//int j = get_global_id(0);
			//int i = get_global_id(1);
	
			int j = (get_group_id(0) * cX + x) * get_local_size(0) + get_local_id(0);
      int i = (get_group_id(1) * cY + y) * get_local_size(1) + get_local_id(1);

			if ((i < ni) && (j < nj))
			{
				int k;
				E[i*nj + j] = 0;
				for(k=0; k < nk; k++)
				{
					E[i * nj + j] += A[i * nk + k] * B[k * nj + j];
				}
			}
		}
	}
}

__kernel void mm3_kernel2(__global DATA_TYPE *C, __global DATA_TYPE *D, __global DATA_TYPE *F, int nj, int nl, int nm) 
{
	int j = get_global_id(0);
	int i = get_global_id(1);
	
	if ((i < nj) && (j < nl))
	{
		int k;
		F[i*nl + j] = 0;
		for(k=0; k < nm; k++)
		{
			F[i * nl + j] += C[i * nm + k] * D[k * nl +j];
		}
	}

}

__kernel void mm3_kernel3(__global DATA_TYPE *E, __global DATA_TYPE *F, __global DATA_TYPE *G, int ni, int nl, int nj) 
{    
	int j = get_global_id(0);
	int i = get_global_id(1);
	
	if ((i < ni) && (j < nl))
	{
		int k;
		G[i*nl + j] = 0;
		for(k=0; k < nj; k++)
		{
			G[i * nl + j] += E[i * nj + k] * F[k * nl + j];
		}
	}
}
