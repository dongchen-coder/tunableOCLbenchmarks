import sys
import os

Inc = "#include \"3DConvolution.cuh\"\n \
#include <polybench.h>\n \
#include <polybenchUtilFuncts.h>\n"

funcBegin = "__global__ void convolution3D_kernel(int ni, int nj, int nk, DATA_TYPE* A, DATA_TYPE* B, int i)\n \
{\n \
DATA_TYPE c11, c12, c13, c21, c22, c23, c31, c32, c33;\n \
	c11 = +2;  c21 = +5;  c31 = -8; \n \
	c12 = -3;  c22 = +6;  c32 = -9; \n \
	c13 = +4;  c23 = +7;  c33 = +10; \n"

funcBody = "if ((i < (_PB_NI-1)) && (j < (_PB_NJ-1)) &&  (k < (_PB_NK-1)) && (i > 0) && (j > 0) && (k > 0)) \n\
	{ \n \
		B[i*(NK * NJ) + j*NK + k] = c11 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]  +  c13 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)] \n\
						 +   c21 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]  +  c23 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)] \n\
						 +   c31 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]  +  c33 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)] \n\
						 +   c12 * A[(i + 0)*(NK * NJ) + (j - 1)*NK + (k + 0)]  +  c22 * A[(i + 0)*(NK * NJ) + (j + 0)*NK + (k + 0)] \n\
						 +   c32 * A[(i + 0)*(NK * NJ) + (j + 1)*NK + (k + 0)]  +  c11 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k + 1)] \n\
						 +   c13 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k + 1)]  +  c21 * A[(i - 1)*(NK * NJ) + (j + 0)*NK + (k + 1)] \n\
						 +   c23 * A[(i + 1)*(NK * NJ) + (j + 0)*NK + (k + 1)]  +  c31 * A[(i - 1)*(NK * NJ) + (j + 1)*NK + (k + 1)] \n\
						 +   c33 * A[(i + 1)*(NK * NJ) + (j + 1)*NK + (k + 1)]; \n \
	}\n"

funcEnd = "}"

index = "int k = (blockIdx.x * CX + x) * blockDim.x + threadIdx.x; \n \
		int j = (blockIdx.y * CY + y) * blockDim.y + threadIdx.y;\n"

for i in [1, 2, 4, 8]:
	for j in [1, 2, 4, 8, 16, 32]:

		f = open('3DConvolution_kernel_test.cu','w')
		f.write(Inc)
		str1 = "#define CX " + str(i) + "\n"
		str1 += "#define CY " + str(j) + "\n"
		f.write(str1)

		for y in range(j):
			for x in range(i):
				f.write(index.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
				f.write(funcBody)

		f.write(funcEnd)
		f.close()

		os.system('make')
		os.system('./3DConvolution.exe')
