import sys
import os


funcBegin = '#include "conf.h"\ntypedef float DATA_TYPE;\n#if defined(cl_khr_fp64)  // Khronos extension available?\n#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n#elif defined(cl_amd_fp64)  // AMD extension available?\n#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n#endif\n__kernel void Convolution3D_kernel(__global DATA_TYPE *A, __global DATA_TYPE *B, int ni, int nj, int nk, int i){\n	int k,j;\n	DATA_TYPE c11, c12, c13, c21, c22, c23, c31, c32, c33;\n    c11 = +2;  c21 = +5;  c31 = -8;\n   c12 = -3;  c22 = +6;  c32 = -9;\n   c13 = +4;  c23 = +7;  c33 = +10;\n'

funcEnd = '}'

index = '	k = (get_group_id(0) * CX + x) * get_local_size(0) + get_local_id(0);\n	j = (get_group_id(1) * CY + y) * get_local_size(1) + get_local_id(1);\n'
code = '	if ((i < (ni - 1)) && (j < (nj - 1)) &&  (k < (nk - 1)) && (i > 0) && (j > 0) && (k > 0)) {\n		B[i*(nk * nj) + j*nk + k] = c11 * A[(i - 1)*(nk * nj) + (j - 1)*nk + (k - 1)]\n	 	+  c13 * A[(i + 1)*(nk * nj) + (j - 1)*nk + (k - 1)]\n		+   c21 * A[(i - 1)*(nk * nj) + (j - 1)*nk + (k - 1)]  +  c23 * A[(i + 1)*(nk * nj) + (j - 1)*nk + (k - 1)]\n		+   c31 * A[(i - 1)*(nk * nj) + (j - 1)*nk + (k - 1)]  +  c33 * A[(i + 1)*(nk * nj) + (j - 1)*nk + (k - 1)]\n		+   c12 * A[(i + 0)*(nk * nj) + (j - 1)*nk + (k + 0)]  +  c22 * A[(i + 0)*(nk * nj) + (j + 0)*nk + (k + 0)]\n		+   c32 * A[(i + 0)*(nk * nj) + (j + 1)*nk + (k + 0)]  +  c11 * A[(i - 1)*(nk * nj) + (j - 1)*nk + (k + 1)]\n		+   c13 * A[(i + 1)*(nk * nj) + (j - 1)*nk + (k + 1)]  +  c21 * A[(i - 1)*(nk * nj) + (j + 0)*nk + (k + 1)]\n		+   c23 * A[(i + 1)*(nk * nj) + (j + 0)*nk + (k + 1)]  +  c31 * A[(i - 1)*(nk * nj) + (j + 1)*nk + (k + 1)]\n		+   c33 * A[(i + 1)*(nk * nj) + (j + 1)*nk + (k + 1)];}\n	else {\n		B[i*(nk * nj) + j*nk + k] = 0;\n	}\n'

os.system('rm -r ~/.nv/ComputeCache/')

for i in [1, 2, 4, 8]:
	for j in [1, 2, 4, 8, 16, 32]:

		f = open('../convolution-3d/conf.h', 'w')
		f.write('#define CX ' + str(i) +'\n')
		f.write('#define CY ' + str(j) + '\n')
		f.close()
		
		f = open('../convolution-3d/3DConvolution.cl','w')
		f_bk = open('../convolution-3d/kernels/3DConvolution'+str(8/i)+'_'+str(32/j)+'.cl', 'w')
		f.write(funcBegin)
		f_bk.write(funcBegin)
		for y in range(j):
			for x in range(i):
				f.write(index.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
				f.write(code)
				f_bk.write(index.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
				f_bk.write(code)
		f.write(funcEnd)
		f_bk.write(funcEnd)
		f.close()	
		f_bk.close()
	
		os.system('make')
		os.system('../convolution-3d/3DConvolution.exe')

		
			
	
