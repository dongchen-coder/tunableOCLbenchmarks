#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include "../utilities/aeol.h"
#include "../utilities/rtd.h"


void gemm_GXYW(float *A, float *B, float *C, float alpha, float beta, int ni, int nj, int nk, int cX, int cY, int widx, int widy, int lidx, int lidy) {

	//printf("wdix %d wdiy %d; cX %d cY %d; lidx %d lidy %d\n", widx, widy, cX, cY, lidx, lidy);

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
	/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int lx = 0; lx < lidx; lx++) {
	/* kernel */
					for (int y = 0; y < cY; y++) {
        				for (int x = 0; x < cX; x++) {
            				int j = (wx * cX + x) * lidx + lx;
            				int i = (wy * cY + y) * lidy + ly;
						
            				if ((i < ni) && (j < nj))
            				{
								//printf("%d %d\n", j, i);
                				C[i * nj + j] *= beta;

								//access(i * nj + j + ni * nk + nk * nj, wy*widx+wx);								
								accessRTD(i * nj + j + ni * nk + nk * nj, wy*widx+wx);

                				int k;
                				for(k=0; k < nk; k++)
                				{
                    				C[i * nj + j] += alpha * A[i * nk + k] * B[k * nj +j];
                					//access(i * nk + k, wy*widx+wx);
									//access(k * nj +j + ni * nk, wy*widx+wx);
									//access(i * nj + j + ni * nk + nk * nj, wy*widx+wx);
									accessRTD(i * nk + k, wy*widx+wx);
                                    accessRTD(k * nj +j + ni * nk, wy*widx+wx);
                                    accessRTD(i * nj + j + ni * nk + nk * nj, wy*widx+wx);
								}
            				}
        				}
    				}
				}
			}
		}
	}
	return;
}

void gemm_CPU(float *A, float *B, float *C, float alpha, float beta, int ni, int nj, int nk) {
	
	int i,j,k;
	
	for (i = 0; i < ni; i++)
	{
		for (j = 0; j < nj; j++)
		{
			C[i * nj + j] *= beta;

			for (k = 0; k < nk; k++)
			{
				C[i * nj + j] += alpha * A[i * nk + k] * B[k * nj + j];
			}
		}
	}
	return;
}

void init_data(float *A, float *B, float *C, float *C_ref, int ni, int nj, int nk) {
	for (int i = 0; i < ni; i++) {
		for (int k= 0; k < nk; k++) {
			A[i * nk + k] = (i * nk + k) % 2;
		}
	}
	for (int k = 0; k < nk; k++) {
		for (int j = 0; j < nj; j++) {
			B[k * nj + j] = (k * nj + j) % 4;
		}
	}
	for (int i = 0; i < ni; i++) {
		for (int j = 0; j < nj; j++) {
			C[i * nj + j] = (i * nj + j) % 8;
			C_ref[i * nj + j] = C[i * nj + j];
		}
	}
	return;
}

bool verify (float * C, float * C_ref, int ni, int nj) {
	for (int i = 0; i < ni; i++) {
		for (int j = 0; j < nj; j++) {
			if (C[i * nj + j] != C_ref[i * nj + j]) {
				//printf("%f-%f\n", C[i * nj + j], C_ref[i * nj + j]);
				printf("i %d, j %d, C %f, C_ref %f\n", i, j, C[i * nj + j], C_ref[i * nj + j]);
				return false;
			}
		}
		//printf("\n");
	}
	return true;
}

int main() {	
	
	float *A;
	float *B;
	float *C;
	float *C_ref;
	int ni, nj, nk;
	float alpha, beta;	

	ni = 32;
	nj = 32;
	nk = 32;

	alpha = 0.3;
	beta = 0.5;

	A = (float *) malloc(sizeof(float) * ni * nk);
	B = (float *) malloc(sizeof(float) * nj * nk);
	C = (float *) malloc(sizeof(float) * ni * nj);
	C_ref = (float *) malloc(sizeof(float) * ni * nj);

	uint64_t gidx, gidy, lidx, lidy;
	
	lidx = 1;
	lidy = 1;
	
	gidx = (uint64_t)ceil(((float)nj) / ((float)lidx)) * lidx;
	gidy = (uint64_t)ceil(((float)ni) / ((float)lidy)) * lidy;

	size_t coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {
	//int cX = 1;
	//int cY = 1;
			resetRTD();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = gidx / cX;
        	globalWorkSizeC[1] = gidy / cY;

			init_data(A, B, C, C_ref,  ni, nj, nk);
			gemm_CPU(A, B, C_ref, alpha, beta, ni, nj, nk);			

			printf("global work size %d, %d local work size %llu, %llu\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);
	
			gemm_GXYW(A, B, C, alpha, beta, ni, nj, nk, cX, cY, globalWorkSizeC[0]/lidx, globalWorkSizeC[1]/lidy, lidx, lidy);
			if (verify(C, C_ref, ni, nj) == false) {
				printf("Result does not MATCH\n");
			}
			
			calculateRTD();
			
		}
	}
	
	return 0;
}

