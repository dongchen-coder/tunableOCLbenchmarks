import sys
import os
import random

Inc = "#include \"3DConvolution.cuh\"\n\
#include <polybench.h>\n\
#include <polybenchUtilFuncts.h>\n"

funcBegin = "__global__ void convolution3D_kernel(int ni, int nj, int nk, DATA_TYPE* A, DATA_TYPE* B, int i)\n \
{\n \
DATA_TYPE c11, c12, c13, c21, c22, c23, c31, c32, c33;\n \
	c11 = +2;  c21 = +5;  c31 = -8; \n \
	c12 = -3;  c22 = +6;  c32 = -9; \n \
	c13 = +4;  c23 = +7;  c33 = +10; \n \
int k, j;\n"

smid = "__device__ uint get_smid(void) { \n\
     uint ret;\n\
     asm(\"mov.u32 %0, %smid;\" : \"=r\"(ret) );\n\
     return ret;\n\
}\n"

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

index = "k = (blockIdx.x * CX + x) * blockDim.x + threadIdx.x; \n \
	j = (blockIdx.y * CY + y) * blockDim.y + threadIdx.y;\n"

mappedIndex = "k = (xmapping[blockIdx.x] * CX + x) * blockDim.x + threadIdx.x; \n \
    j = (ymapping[blockIdx.y] * CY + y) * blockDim.y + threadIdx.y;\n"

#256
#for i in [1, 2, 4, 8]:
#	for j in [1, 2, 4, 8, 16, 32]:

#2048

#for i in [1, 2, 4, 8, 16, 32, 64]:
#	for j in [1, 2, 4, 8, 16, 32, 64, 128, 256]:


#for i in [1]:
#	for j in [1]:

os.system('rm summary_org.txt')

for k in range(5):
	
		i = 1
                j = 1
                xmapping = range(64/i);
                ymapping = range(256/j);
		
		f = open('3DConvolution_kernel.cu','w')
                f.write(Inc)
                str1 = "#define CX " + str(i) + "\n"
                str1 += "#define CY " + str(j) + "\n"
                f.write(str1)

		print xmapping
                print ','.join(map(str, xmapping))
                f.write('__device__ int xmapping['+str(64/i)+'] = {'+','.join(map(str, xmapping)) + '};\n')
                f.write('__device__ int ymapping['+str(256/j)+'] = {'+','.join(map(str, ymapping)) + '};\n')

		f.write(funcBegin)

                for y in range(j):
                        for x in range(i):
                                #f.write(index.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
                                f.write(mappedIndex.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
                                f.write(funcBody)

		f.write(funcEnd)
                f.close()

                os.system('make')
                conf = str(64/i) + "_" + str(256/j) + "_" + str(k) + "_resOrg.txt"
                os.system('rm ' + conf)
                #os.system('nvprof --log-file ' + conf + ' --events all --metrics all ./3DConvolution.exe')
                os.system('nvprof --log-file ' + conf + ' --metrics l2_l1_read_hit_rate,achieved_occupancy ./3DConvolution.exe')
                os.system('cat ' + conf + '| grep l2_l1 >> summary_org.txt')


#os.system('rm summary.txt')

for k in range(0):
	
		i = 1
		j = 1
		xmapping = range(64/i);
		ymapping = range(256/j);
		random.shuffle(xmapping)
		random.shuffle(ymapping)
	

		f = open('3DConvolution_kernel.cu','w')
		f.write(Inc)
		str1 = "#define CX " + str(i) + "\n"
		str1 += "#define CY " + str(j) + "\n"
		f.write(str1)
	

		print xmapping
		print ','.join(map(str, xmapping))
		f.write('__device__ int xmapping['+str(64/i)+'] = {'+','.join(map(str, xmapping)) + '};\n')		
		f.write('__device__ int ymapping['+str(256/j)+'] = {'+','.join(map(str, ymapping)) + '};\n')

		#f.write(smid)

		f.write(funcBegin)

		for y in range(j):
			for x in range(i):
				#f.write(index.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
				f.write(mappedIndex.replace(' x)',' '+str(x)+')').replace(' y)',' '+str(y)+')'))
				f.write(funcBody)

		#f.write("smids[(blockIdx.y * blockDimx + blockIdx.x) * threadDim.x * threadDim.y + threadIdx.y * threadDim.x + threadIdx.x] = get_smid();")

		f.write(funcEnd)
		f.close()

		os.system('make')
		conf = str(64/i) + "_" + str(256/j) + "_" + str(k) + "_res.txt"
		os.system('rm ' + conf)
		#os.system('nvprof --log-file ' + conf + ' --events all --metrics all ./3DConvolution.exe')
		os.system('nvprof --log-file ' + conf + ' --metrics l2_l1_read_hit_rate,achieved_occupancy ./3DConvolution.exe')
		os.system('cat ' + conf + '| grep l2_l1 >> summary.txt')

		#os.system('./3DConvolution.exe')
