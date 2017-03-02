#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define NI 512
#define NJ 512
#define NK 512

#define DIM_LOCAL_WORK_GROUP_X 32
#define DIM_LOCAL_WORK_GROUP_Y 8

void gemm_GXYW(float *A, float *B, float *C, float alpha, float beta, int cX, int cY, int widx, int widy, int lidx, int lidy, void (*access)(uint64_t addr, uint64_t wgid)) {
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
						
            				if ((i < NI) && (j < NJ))
            				{
								//printf("%d %d\n", j, i);
                				C[i * NJ + j] *= beta;

								(*access)(i * NJ + j + NI * NK + NK * NJ, wy * widx + wx);

                				int k;
                				for(k=0; k < NK; k++)
                				{
                    				C[i * NJ + j] += alpha * A[i * NK + k] * B[k * NJ +j];
									(*access)(i * NK + k, wy * widx + wx);
                                    (*access)(k * NJ +j + NI * NK, wy * widx + wx);
                                    (*access)(i * NJ + j + NI * NK + NK * NJ, wy * widx + wx);
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

void gemm_CPU(float *A, float *B, float *C, float alpha, float beta) {
	
	int i,j,k;
	
	for (i = 0; i < NI; i++)
	{
		for (j = 0; j < NJ; j++)
		{
			C[i * NJ + j] *= beta;

			for (k = 0; k < NK; k++)
			{
				C[i * NJ + j] += alpha * A[i * NK + k] * B[k * NJ + j];
			}
		}
	}
	return;
}

void init_data(float *A, float *B, float *C, float *C_ref) {
	for (int i = 0; i < NI; i++) {
		for (int k= 0; k < NK; k++) {
			A[i * NK + k] = (i * NK + k) % 2;
		}
	}
	for (int k = 0; k < NK; k++) {
		for (int j = 0; j < NJ; j++) {
			B[k * NJ + j] = (k * NJ + j) % 4;
		}
	}
	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NJ; j++) {
			C[i * NJ + j] = (i * NJ + j) % 8;
			C_ref[i * NJ + j] = C[i * NJ + j];
		}
	}
	return;
}

void verify (float * C, float * C_ref) {
	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NJ; j++) {
			if (C[i * NJ + j] != C_ref[i * NJ + j]) {
				cout << "Error in kernel " << i << " " << j << " " << C[i * NJ + j] << " " << C_ref[i * NJ + j] << endl;
				return;
			}
		}
	}
	return;
}

int gemm_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void), int cX, int cY) {	
	
	float *A;
	float *B;
	float *C;
	float *C_ref;
	float alpha, beta;	

	alpha = 0.3;
	beta = 0.5;

	A = (float *) malloc(sizeof(float) * NI * NK);
	B = (float *) malloc(sizeof(float) * NJ * NK);
	C = (float *) malloc(sizeof(float) * NI * NJ);
	C_ref = (float *) malloc(sizeof(float) * NI * NJ);

	uint64_t gidx, gidy, lidx, lidy;
	
	lidx = DIM_LOCAL_WORK_GROUP_X;
	lidy = DIM_LOCAL_WORK_GROUP_Y;
	
	gidx = (uint64_t)ceil(((float)NJ) / ((float)lidx)) * lidx;
	gidy = (uint64_t)ceil(((float)NI) / ((float)lidy)) * lidy;

	size_t coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;


	if (cX <= coalescingMax[0] && cY <= coalescingMax[1]) {		
			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
        	globalWorkSizeC[1] = (gidy / cY) / lidy;

			init_data(A, B, C, C_ref);
			gemm_CPU(A, B, C_ref, alpha, beta);			

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;	

			gemm_GXYW(A, B, C, alpha, beta, cX, cY, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, access);
			
			(*calculate)();
			
			(*dump)();
	} else {
		cout << "No such config:" << cX << " " << cY << " " << coalescingMax[0] << " " << coalescingMax[1] << endl;
	}
	
	return 0;
}

